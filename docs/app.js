'use strict';

/* ═══════════════════════════════════════════════════════════════
   CODEXION VISUALIZER — APPLICATION
   ═══════════════════════════════════════════════════════════════ */

const CODER_COLORS = [
  '#818cf8','#4ade80','#c084fc','#fbbf24','#f87171',
  '#2dd4bf','#f472b6','#60a5fa','#a3e635','#e879f9',
  '#fb923c','#34d399',
];

/* ═══════════════════════════════════════════════════════════════
   INPUT PARSER — parses "N burn compile debug refactor compiles cd sched"
   ═══════════════════════════════════════════════════════════════ */

class InputParser {
  static parse(text) {
    const t = text.trim();
    if (!t) return { params: null, error: 'Empty configuration input' };
    const parts = t.split(/\s+/);
    if (parts.length !== 8)
      return { params: null, error: `Expected 8 parameters, got ${parts.length}` };
    const nums = parts.slice(0, 7).map(Number);
    for (let i = 0; i < 7; i++) {
      if (isNaN(nums[i]) || nums[i] < 0)
        return { params: null, error: `Parameter ${i + 1} ("${parts[i]}") must be a non-negative integer` };
    }
    const sched = parts[7].toLowerCase();
    if (sched !== 'fifo' && sched !== 'edf')
      return { params: null, error: `Scheduler must be "fifo" or "edf", got "${parts[7]}"` };
    if (nums[0] < 1) return { params: null, error: 'Number of coders must be >= 1' };
    if (nums[5] < 1) return { params: null, error: 'Number of compiles required must be >= 1' };
    return {
      params: {
        numCoders: nums[0],
        burnout: nums[1],
        compile: nums[2],
        debug: nums[3],
        refactor: nums[4],
        compilesReq: nums[5],
        cooldown: nums[6],
        scheduler: sched,
      },
      error: null,
    };
  }
}

/* ═══════════════════════════════════════════════════════════════
   OUTPUT PARSER — parses simulation log lines
   ═══════════════════════════════════════════════════════════════ */

class OutputParser {
  static RE = /^(\d+)\s+(\d+)\s+(.+)$/;
  static EVENT_MAP = {
    'has taken a dongle': 'taken',
    'is compiling': 'compiling',
    'is debugging': 'debugging',
    'is refactoring': 'refactoring',
    'burned out': 'burned',
  };

  static parse(text, params) {
    const lines = text.split('\n');
    const events = [];
    const errors = [];
    const coderIds = new Set();

    lines.forEach((raw, idx) => {
      const line = raw.trim();
      if (!line) return;
      const m = line.match(OutputParser.RE);
      if (!m) { errors.push({ line: idx + 1, sev: 'error', type: 'format', msg: 'Unrecognized line format', raw: line }); return; }
      const time = parseInt(m[1], 10);
      const coderId = parseInt(m[2], 10);
      const msg = m[3];

      if (time < 0 || !/^\d+$/.test(m[1]))
        { errors.push({ line: idx + 1, sev: 'error', type: 'format', msg: `Invalid timestamp "${m[1]}"`, raw: line }); return; }
      if (isNaN(coderId) || coderId < 1)
        { errors.push({ line: idx + 1, sev: 'error', type: 'format', msg: `Invalid coder ID "${m[2]}"`, raw: line }); return; }
      if (params && coderId > params.numCoders)
        { errors.push({ line: idx + 1, sev: 'error', type: 'format', msg: `Coder ID ${coderId} exceeds configured ${params.numCoders} coders`, raw: line }); return; }

      const type = OutputParser.EVENT_MAP[msg];
      if (!type)
        { errors.push({ line: idx + 1, sev: 'error', type: 'format', msg: `Unknown event "${msg}"`, raw: line }); return; }

      coderIds.add(coderId);
      events.push({ time, coderId, type, line: idx + 1 });
    });

    events.sort((a, b) => a.time - b.time || a.line - b.line);
    return { events, errors, coderIds: [...coderIds].sort((a, b) => a - b) };
  }
}

/* ═══════════════════════════════════════════════════════════════
   VALIDATOR — validates output against simulation spec rules only
   ═══════════════════════════════════════════════════════════════ */

class Validator {
  constructor(params, events) {
    this.p = params;
    this.events = events;
    this.issues = [];
    this.errorLines = new Set();
  }

  validate() {
    if (!this.p) return this.issues;
    this._checkStateTransitions();
    this._checkCompileTimestamps();
    this._checkTiming();
    this._checkBurnoutAccuracy();
    this._checkDongleConflicts();
    this._checkCooldown();
    this._checkDuplicates();
    return this.issues;
  }

  _add(sev, type, line, coderId, msg, detail) {
    this.issues.push({ sev, type, line, coderId, msg, detail: detail || '' });
    if (line != null) this.errorLines.add(line);
  }

  _perCoder() {
    const m = {};
    for (const e of this.events) {
      if (!m[e.coderId]) m[e.coderId] = [];
      m[e.coderId].push(e);
    }
    return m;
  }

  /* Spec rule: valid lifecycle is
     idle → taken → taken → compiling → debugging → refactoring → taken (repeat)
     burned out is terminal. */
  _checkStateTransitions() {
    const VALID = {
      idle:        ['taken'],
      taken_1:     ['taken', 'compiling'],
      taken_2:     ['compiling'],
      compiling:   ['debugging'],
      debugging:   ['refactoring'],
      refactoring: ['taken'],
      burned:      [],
    };
    for (const [cid, evts] of Object.entries(this._perCoder())) {
      let st = 'idle', n = 0;
      for (const ev of evts) {
        if (!(VALID[st] || []).includes(ev.type)) {
          this._add('error', 'state', ev.line, parseInt(cid),
            'Invalid state transition',
            `Coder ${cid} in state "${st}", got "${ev.type}" at ${ev.time}ms`);
        }
        if (ev.type === 'taken') { n++; st = n % 2 ? 'taken_1' : 'taken_2'; }
        else if (ev.type === 'compiling')   { st = 'compiling'; }
        else if (ev.type === 'debugging')   { st = 'debugging'; n = 0; }
        else if (ev.type === 'refactoring') { st = 'refactoring'; }
        else if (ev.type === 'burned')      { st = 'burned'; }
      }
    }
  }

  /* Spec rule: each cycle prints exactly 2 "has taken a dongle" + "is compiling"
     all at the same timestamp via log_compiling(). */
  _checkCompileTimestamps() {
    for (const [cid, evts] of Object.entries(this._perCoder())) {
      let cnt = 0, t0 = null;
      for (const ev of evts) {
        if (ev.type === 'taken') {
          if (t0 === null) t0 = ev.time;
          if (ev.time !== t0) {
            if (cnt !== 2) this._add('error', 'dongle', ev.line, parseInt(cid),
              'Invalid dongle takes per cycle',
              `Coder ${cid}: ${cnt} take(s) in previous cycle, expected 2`);
            t0 = ev.time; cnt = 0;
          }
          cnt++;
        } else if (ev.type === 'compiling') {
          if (cnt !== 2) this._add('error', 'dongle', ev.line, parseInt(cid),
            'Invalid dongle takes per cycle',
            `Coder ${cid}: ${cnt} take(s) before compile, expected 2`);
          if (t0 !== null && ev.time !== t0) this._add('error', 'dongle', ev.line, parseInt(cid),
            'Compile timestamp mismatch',
            `Coder ${cid}: compile at ${ev.time}ms but takes at ${t0}ms`);
          cnt = 0; t0 = null;
        } else if (ev.type === 'debugging' || ev.type === 'burned') {
          cnt = 0; t0 = null;
        }
      }
    }
  }

  /* Spec rule: compile/debug/refactor phases must last >= configured durations. */
  _checkTiming() {
    for (const [cid, evts] of Object.entries(this._perCoder())) {
      let cStart = null, dStart = null, rStart = null;
      for (const ev of evts) {
        if (ev.type === 'compiling') { cStart = ev.time; }
        else if (ev.type === 'debugging') {
          if (cStart !== null && ev.time - cStart < this.p.compile)
            this._add('error', 'timing', ev.line, parseInt(cid),
              'Compile phase too short',
              `Coder ${cid}: ${ev.time - cStart}ms, expected ≥${this.p.compile}ms`);
          dStart = ev.time; cStart = null;
        } else if (ev.type === 'refactoring') {
          if (dStart !== null && ev.time - dStart < this.p.debug)
            this._add('error', 'timing', ev.line, parseInt(cid),
              'Debug phase too short',
              `Coder ${cid}: ${ev.time - dStart}ms, expected ≥${this.p.debug}ms`);
          rStart = ev.time; dStart = null;
        } else if (ev.type === 'taken' && rStart !== null) {
          if (ev.time - rStart < this.p.refactor)
            this._add('error', 'timing', ev.line, parseInt(cid),
              'Refactor phase too short',
              `Coder ${cid}: ${ev.time - rStart}ms, expected ≥${this.p.refactor}ms`);
          rStart = null;
        }
      }
    }
  }

  /* Spec rule: burnout fires when get_time_ms() > last_compile_t + burnout_t.
     If a burnout event exists, it must be within 10ms of the computed deadline. */
  _checkBurnoutAccuracy() {
    if (this.p.burnout <= 0) return;
    for (const [cid, evts] of Object.entries(this._perCoder())) {
      let lastCompile = 0;
      for (const ev of evts) {
        if (ev.type === 'compiling') lastCompile = ev.time;
        if (ev.type === 'burned') {
          const deadline = lastCompile + this.p.burnout;
          const diff = Math.abs(ev.time - deadline);
          if (diff > 10) this._add('error', 'burnout', ev.line, parseInt(cid),
            'Burnout timing inaccurate',
            `Coder ${cid}: burned at ${ev.time}ms, deadline ${deadline}ms (diff=${diff}ms, allowed ≤10ms)`);
          break;
        }
      }
    }
  }

  /* Spec rule: two coders cannot hold the same dongle simultaneously.
     Dongle assignment (sim_init.c): even i → D[i],D[(i+1)%n]; odd i → swapped.
     Hold starts at "compiling", ends at "debugging". */
  _checkDongleConflicts() {
    const n = this.p.numCoders;
    const holds = Array.from({ length: n }, () => []);

    for (const ev of this.events) {
      if (ev.type !== 'compiling' && ev.type !== 'debugging') continue;
      const i = ev.coderId - 1;
      const ds = [i, (i + 1) % n];
      if (ev.type === 'compiling') {
        for (const d of ds) holds[d].push({ s: ev.time, e: null, c: ev.coderId, l: ev.line });
      } else {
        for (const d of ds) {
          for (let j = holds[d].length - 1; j >= 0; j--) {
            if (holds[d][j].c === ev.coderId && holds[d][j].e === null) {
              holds[d][j].e = ev.time; break;
            }
          }
        }
      }
    }

    for (let d = 0; d < n; d++) {
      const h = holds[d];
      for (let a = 0; a < h.length; a++) {
        for (let b = a + 1; b < h.length; b++) {
          if (h[a].s < h[b].e && h[b].s < h[a].e)
            this._add('error', 'dongle', h[b].l, h[b].c,
              `Dongle D${d + 1} conflict`,
              `Coder ${h[b].c} held D${d + 1} at ${h[b].s}ms while coder ${h[a].c} still held it`);
        }
      }
    }
  }

  /* Spec rule: after put_dongles(), dongle.available_at = now + cooldown.
     Any subsequent take must respect this per-dongle cooldown. */
  _checkCooldown() {
    if (this.p.cooldown <= 0) return;
    const n = this.p.numCoders;
    const rels = Array.from({ length: n }, () => []);

    for (const ev of this.events) {
      if (ev.type === 'debugging') {
        const i = ev.coderId - 1;
        rels[i].push(ev.time);
        rels[(i + 1) % n].push(ev.time);
      }
    }

    for (let d = 0; d < n; d++) {
      const dr = rels[d].sort((a, b) => a - b);
      for (const ev of this.events) {
        if (ev.type !== 'compiling') continue;
        const i = ev.coderId - 1;
        if (d !== i && d !== (i + 1) % n) continue;
        for (let j = dr.length - 1; j >= 0; j--) {
          if (dr[j] < ev.time) {
            const gap = ev.time - dr[j];
            if (gap < this.p.cooldown) this._add('error', 'dongle', ev.line, ev.coderId,
              `Dongle D${d + 1} cooldown violated`,
              `Coder ${ev.coderId} took D${d + 1} at ${ev.time}ms, only ${gap}ms after release (cooldown=${this.p.cooldown}ms)`);
            break;
          }
        }
      }
    }
  }

  /* The spec requires exactly 2 "has taken a dongle" at the same timestamp
     per cycle (from log_compiling()). Pairs are expected, not duplicates. */
  _checkDuplicates() {
    const seen = new Map();
    for (const ev of this.events) {
      const k = `${ev.coderId}:${ev.type}:${ev.time}`;
      if (seen.has(k)) {
        const prev = seen.get(k);
        // Allow exactly 2 "taken" at same time/coder (spec-mandated pair)
        if (ev.type === 'taken' && prev.count === 1) {
          prev.count = 2;
        } else {
          this._add('error', 'duplicate', ev.line, ev.coderId,
            'Duplicate event',
            `Coder ${ev.coderId} "${ev.type}" at ${ev.time}ms (lines ${prev.line} & ${ev.line})`);
        }
      } else {
        seen.set(k, { line: ev.line, count: 1 });
      }
    }
  }
}

/* ═══════════════════════════════════════════════════════════════
   SIMULATION ENGINE
   ═══════════════════════════════════════════════════════════════ */

class SimulationEngine {
  constructor(parsed) {
    this.events = parsed.events;
    this.coderList = parsed.coderIds;
    this.numCoders = this.coderList.length;
    this.duration = 0;
    this.coderStates = {};
    this.dongleMap = {};
    this.compileCounts = {};
    this.burnedSet = new Set();
    this.completedSet = new Set();
    this._build();
  }

  _build() {
    const perCoder = {};
    for (const id of this.coderList) { perCoder[id] = []; this.compileCounts[id] = 0; }
    for (const ev of this.events) {
      perCoder[ev.coderId].push(ev);
      if (ev.time > this.duration) this.duration = ev.time;
    }
    if (this.duration === 0) this.duration = 1;
    for (const id of this.coderList) this.coderStates[id] = this._intervals(perCoder[id], id);
    this._buildDongleMap(perCoder);
  }

  _intervals(evts, cid) {
    if (!evts.length) return [];
    const ivs = [];
    let st = 'idle', t0 = null;
    const flush = (t) => { if (t0 !== null && t > t0) ivs.push({ start: t0, end: t, state: st }); };
    for (const ev of evts) {
      let ns = st;
      if (ev.type === 'taken' && st !== 'compiling') ns = 'compiling';
      else if (ev.type === 'compiling') { ns = 'compiling'; this.compileCounts[cid]++; }
      else if (ev.type === 'debugging') ns = 'debugging';
      else if (ev.type === 'refactoring') ns = 'refactoring';
      else if (ev.type === 'burned') { ns = 'burned'; this.burnedSet.add(cid); }
      if (ns !== st) { flush(ev.time); st = ns; t0 = ev.time; }
    }
    if (t0 !== null && t0 < this.duration) ivs.push({ start: t0, end: this.duration, state: st });
    if (st !== 'burned' && st !== 'idle') {
      this.completedSet.add(cid);
      if (ivs.length) ivs[ivs.length - 1].state = 'done';
    }
    return ivs;
  }

  _buildDongleMap(perCoder) {
    const n = this.numCoders;
    for (let i = 0; i < n; i++) {
      const cid = this.coderList[i];
      const d1 = i, d2 = (i + 1) % n;
      for (const ev of perCoder[cid]) {
        if (ev.type === 'compiling') { this._setOwn(d1, ev.time, cid); this._setOwn(d2, ev.time, cid); }
        if (ev.type === 'debugging') { this._relOwn(d1, ev.time); this._relOwn(d2, ev.time); }
      }
    }
  }

  _setOwn(d, t, c) {
    if (!this.dongleMap[d]) this.dongleMap[d] = [];
    this.dongleMap[d].push({ start: t, end: null, owner: c });
  }

  _relOwn(d, t) {
    const e = this.dongleMap[d];
    if (!e) return;
    for (let i = e.length - 1; i >= 0; i--) {
      if (e[i].owner !== null && e[i].end === null) { e[i].end = t; break; }
    }
  }

  getDongleState(time) {
    const res = [];
    for (let d = 0; d < this.numCoders; d++) {
      let owner = null;
      for (const e of (this.dongleMap[d] || [])) {
        if (time >= e.start && (e.end === null || time < e.end)) { owner = e.owner; break; }
      }
      res.push({ id: d, owner });
    }
    return res;
  }
}

/* ═══════════════════════════════════════════════════════════════
   TIMELINE RENDERER
   ═══════════════════════════════════════════════════════════════ */

class TimelineRenderer {
  constructor(engine, errorLines) {
    this.engine = engine;
    this.errorLines = errorLines || new Set();
    this.pixelsPerMs = 0;
    this.trackWidth = 0;
    this.onCoderClick = null;
    this.onSeek = null;
    this.onErrHover = null;
    this.axisEl = document.getElementById('tl-axis');
    this.bodyEl = document.getElementById('tl-body');
    this.indEl = document.getElementById('tl-indicator');
    this.dongleEl = document.getElementById('dongle-grid');
    this.logEl = document.getElementById('log-body');
  }

  render() {
    this._scale();
    this._axis();
    this._rows();
    this._dongles(0);
    this._log();
    this._indicator(0);
    this._interact();
  }

  _scale() {
    const w = this.axisEl.parentElement;
    const lw = 110;
    const avail = Math.max(w.clientWidth - lw - 24, 500);
    this.pixelsPerMs = Math.max(0.35, avail / this.engine.duration);
    this.trackWidth = Math.max(avail, this.engine.duration * this.pixelsPerMs);
  }

  _axis() {
    this.axisEl.innerHTML = '';
    const tw = this.trackWidth + 110;
    this.axisEl.style.minWidth = tw + 'px';
    const d = this.engine.duration;
    const iv = this._nice(d);
    const hold = document.createElement('div');
    hold.style.cssText = `width:110px;min-width:110px;border-left:none`;
    this.axisEl.appendChild(hold);
    for (let t = 0; t <= d; t += iv) {
      const tk = document.createElement('div');
      tk.className = 'axis-tick';
      tk.style.width = (iv * this.pixelsPerMs) + 'px';
      const s = document.createElement('span');
      s.textContent = t + 'ms';
      tk.appendChild(s);
      this.axisEl.appendChild(tk);
    }
  }

  _nice(d) {
    const ts = [50, 100, 200, 250, 500, 1000, 2000, 5000];
    for (const t of ts) if (d / t <= Math.max(Math.floor(d / 50), 8)) return t;
    return Math.ceil(d / 10 / 100) * 100 || 100;
  }

  _rows() {
    this.bodyEl.innerHTML = '';
    this.bodyEl.style.minWidth = (this.trackWidth + 110) + 'px';
    for (let ci = 0; ci < this.engine.coderList.length; ci++) {
      const cid = this.engine.coderList[ci];
      const color = CODER_COLORS[ci % CODER_COLORS.length];
      const ivs = this.engine.coderStates[cid] || [];
      const cc = this.engine.compileCounts[cid];
      const burned = this.engine.burnedSet.has(cid);
      const done = this.engine.completedSet.has(cid);
      const hasErr = [...this.errorLines].some(l => this.engine.events.some(e => e.line === l && e.coderId === cid));

      const row = document.createElement('div');
      row.className = 'tl-row fade-in' + (hasErr ? ' has-error' : '');
      row.style.animationDelay = (ci * 25) + 'ms';
      row.dataset.cid = cid;

      const lbl = document.createElement('div');
      lbl.className = 'row-lbl';
      lbl.innerHTML = `<span class="row-id" style="color:${color}">C${cid}</span><span class="row-badge">${cc} comp</span>`;
      if (burned) lbl.innerHTML += `<span class="row-icon" title="Burned out">\uD83D\uDD25</span>`;
      else if (done) lbl.innerHTML += `<span class="row-icon" title="Completed">\u2705</span>`;
      row.appendChild(lbl);

      const track = document.createElement('div');
      track.className = 'row-track';

      for (const iv of ivs) {
        const left = iv.start * this.pixelsPerMs;
        const width = Math.max((iv.end - iv.start) * this.pixelsPerMs, 2);
        const bar = document.createElement('div');
        bar.className = 'bar ' + iv.state;
        bar.style.left = left + 'px';
        bar.style.width = width + 'px';
        if (width > 36) {
          const lb = document.createElement('span');
          lb.className = 'bar-label';
          lb.textContent = iv.state.charAt(0).toUpperCase() + iv.state.slice(1);
          bar.appendChild(lb);
        }
        bar.title = `${iv.state} ${iv.start}–${iv.end}ms (${iv.end - iv.start}ms)`;
        track.appendChild(bar);
      }

      for (const ev of this.engine.events) {
        if (ev.coderId !== cid) continue;
        if (!this.errorLines.has(ev.line)) continue;
        const mk = document.createElement('div');
        mk.className = 'bar-err';
        mk.style.left = (ev.time * this.pixelsPerMs - 1) + 'px';
        mk.title = `Line ${ev.line}: validation error`;
        mk.dataset.line = ev.line;
        if (this.onErrHover) {
          mk.addEventListener('mouseenter', () => this.onErrHover(ev.line, true));
          mk.addEventListener('mouseleave', () => this.onErrHover(ev.line, false));
        }
        track.appendChild(mk);
      }

      row.appendChild(track);
      row.addEventListener('click', () => this.onCoderClick && this.onCoderClick(cid));
      this.bodyEl.appendChild(row);
    }
    this.indEl.style.left = '110px';
    this.indEl.style.height = this.bodyEl.scrollHeight + 'px';
  }

  _dongles(time) {
    const own = this.engine.getDongleState(time);
    this.dongleEl.innerHTML = '';
    for (const d of own) {
      const card = document.createElement('div');
      card.className = 'dongle' + (d.owner ? ' locked' : '');
      const ico = d.owner ? '\uD83D\uDD12' : '\uD83D\uDD13';
      const col = d.owner ? CODER_COLORS[this.engine.coderList.indexOf(d.owner) % CODER_COLORS.length] : '#52525b';
      card.innerHTML = `<span class="dongle-ico">${ico}</span><span class="dongle-id">D${d.id + 1}</span><span class="dongle-st" style="color:${col}">${d.owner ? 'C' + d.owner : 'Free'}</span>`;
      this.dongleEl.appendChild(card);
    }
  }

  _log() {
    this.logEl.innerHTML = '';
    const errSet = this.errorLines;
    for (const ev of this.engine.events) {
      const en = document.createElement('div');
      en.className = 'log-entry' + (errSet.has(ev.line) ? ' has-err' : '');
      en.dataset.time = ev.time;
      en.dataset.line = ev.line;
      const msgs = { taken: 'has taken a dongle', compiling: 'is compiling', debugging: 'is debugging', refactoring: 'is refactoring', burned: 'burned out' };
      const ci = this.engine.coderList.indexOf(ev.coderId);
      en.innerHTML = `<span class="log-t">${ev.time}ms</span><span class="log-c" style="color:${CODER_COLORS[ci % CODER_COLORS.length]}">C${ev.coderId}</span><span class="log-e ev-${ev.type}">${msgs[ev.type]}</span>`;
      this.logEl.appendChild(en);
    }
  }

  _indicator(t) {
    this.indEl.style.left = (110 + t * this.pixelsPerMs) + 'px';
  }

  _interact() {
    this.bodyEl.addEventListener('click', (e) => {
      if (!this.onSeek) return;
      const r = this.bodyEl.getBoundingClientRect();
      const x = e.clientX - r.left - 110;
      if (x < 0) return;
      this.onSeek(Math.max(0, Math.min(x / this.pixelsPerMs, this.engine.duration)));
    });
  }

  update(t) {
    this._indicator(t);
    this._dongles(t);
    this._hlLog(t);
  }

  _hlLog(t) {
    let last = null;
    for (const el of this.logEl.children) {
      const v = parseInt(el.dataset.time);
      if (Math.abs(v - t) <= 20) { el.classList.add('hl'); last = el; }
      else el.classList.remove('hl');
    }
    if (last) last.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
  }

  resize() { this._scale(); this._axis(); this._rows(); }
}

/* ═══════════════════════════════════════════════════════════════
   SIMULATION PLAYER
   ═══════════════════════════════════════════════════════════════ */

class SimulationPlayer {
  constructor(dur) {
    this.max = dur; this.time = 0; this.speed = 1;
    this.playing = false; this._raf = null; this._last = null;
    this.onTick = null; this.onEnd = null;
  }
  play() {
    if (this.playing) return;
    if (this.time >= this.max) this.time = 0;
    this.playing = true; this._last = performance.now(); this._loop();
  }
  pause() { this.playing = false; if (this._raf) { cancelAnimationFrame(this._raf); this._raf = null; } }
  toggle() { this.playing ? this.pause() : this.play(); }
  seek(t) { this.time = Math.max(0, Math.min(t, this.max)); if (this.onTick) this.onTick(this.time); }
  goStart() { this.seek(0); } goEnd() { this.seek(this.max); }
  stepFwd() { this.seek(this.time + 50); } stepBack() { this.seek(this.time - 50); }
  setSpeed(s) { this.speed = Math.max(0.25, Math.min(5, s)); }
  _loop() {
    if (!this.playing) return;
    const now = performance.now(), dt = now - this._last; this._last = now;
    this.time += dt * this.speed;
    if (this.time >= this.max) {
      this.time = this.max; this.playing = false;
      if (this.onTick) this.onTick(this.time);
      if (this.onEnd) this.onEnd();
      return;
    }
    if (this.onTick) this.onTick(this.time);
    this._raf = requestAnimationFrame(() => this._loop());
  }
}

/* ═══════════════════════════════════════════════════════════════
   APPLICATION
   ═══════════════════════════════════════════════════════════════ */

class App {
  constructor() {
    this.engine = null;
    this.renderer = null;
    this.player = null;
    this.params = null;
    this.$ = (id) => document.getElementById(id);
    this._bind();
  }

  _bind() {
    this.$('parse-btn').addEventListener('click', () => this._parse());
    this.$('file-upload').addEventListener('change', (e) => this._file(e));
    this.$('load-example').addEventListener('click', () => this._example());
    this.$('btn-play').addEventListener('click', () => this.player && this.player.toggle());
    this.$('btn-start').addEventListener('click', () => this.player && this.player.goStart());
    this.$('btn-end').addEventListener('click', () => this.player && this.player.goEnd());
    this.$('btn-step-back').addEventListener('click', () => this.player && this.player.stepBack());
    this.$('btn-step-fwd').addEventListener('click', () => this.player && this.player.stepFwd());
    this.$('speed-slider').addEventListener('input', (e) => {
      const s = parseFloat(e.target.value);
      this.$('speed-value').textContent = s + 'x';
      if (this.player) this.player.setSpeed(s);
    });
    this.$('scrubber').addEventListener('input', (e) => {
      if (!this.engine) return;
      const t = (parseFloat(e.target.value) / 100) * this.engine.duration;
      if (this.player) { this.player.pause(); this.player.seek(t); }
    });
    document.addEventListener('keydown', (e) => this._key(e));
    window.addEventListener('resize', () => { if (this.renderer) this.renderer.resize(); });
  }

  _key(e) {
    if (['TEXTAREA', 'INPUT'].includes(e.target.tagName)) return;
    if (e.key === ' ') { e.preventDefault(); this.player && this.player.toggle(); }
    else if (e.key === 'ArrowRight') { e.preventDefault(); this.player && this.player.stepFwd(); }
    else if (e.key === 'ArrowLeft') { e.preventDefault(); this.player && this.player.stepBack(); }
    else if (e.key === 'Home') { e.preventDefault(); this.player && this.player.goStart(); }
    else if (e.key === 'End') { e.preventDefault(); this.player && this.player.goEnd(); }
  }

  _example() {
    this.$('params-input').value = '4 800 200 200 200 4 200 fifo';
    this.$('output-input').value = EXAMPLE_OUTPUT;
    this._parse();
  }

  _file(e) {
    const f = e.target.files[0]; if (!f) return;
    const r = new FileReader();
    r.onload = (ev) => { this.$('output-input').value = ev.target.result; this._parse(); };
    r.readAsText(f); e.target.value = '';
  }

  _parse() {
    const paramsText = this.$('params-input').value.trim();
    const outputText = this.$('output-input').value.trim();
    if (!outputText) return;

    let params = null;
    let paramError = null;
    if (paramsText) {
      const pr = InputParser.parse(paramsText);
      if (pr.error) paramError = pr.error;
      else params = pr.params;
    }

    const parsed = OutputParser.parse(outputText, params);

    let validatorIssues = [];
    let errorLines = new Set();
    if (params) {
      const v = new Validator(params, parsed.events);
      validatorIssues = v.validate();
      errorLines = v.errorLines;
    }

    const allErrors = [...parsed.errors];
    for (const iss of validatorIssues) {
      if (iss.line != null) errorLines.add(iss.line);
    }

    if (parsed.events.length === 0 && allErrors.length === 0 && !paramError) return;

    if (paramError) {
      allErrors.unshift({ line: 0, sev: 'error', type: 'format', msg: `Config: ${paramError}`, raw: paramsText });
    }

    this.params = params;
    this.engine = new SimulationEngine(parsed);
    this._stats(params);
    this._showViz();

    this.renderer = new TimelineRenderer(this.engine, errorLines);
    this.renderer.onCoderClick = (cid) => this._detail(cid);
    this.renderer.onSeek = (t) => { if (this.player) { this.player.pause(); this.player.seek(t); } };
    this.renderer.onErrHover = (line, show) => this._hoverErr(line, show);
    this.renderer.render();

    this.player = new SimulationPlayer(this.engine.duration);
    this.player.onTick = (t) => this._tick(t);
    this.player.onEnd = () => this._playBtn();
    this.$('scrubber-total').textContent = this.engine.duration + ' ms';
    this.$('scrubber').value = 0;
    this._playBtn();
    this._validation(validatorIssues, allErrors);
  }

  _showViz() {
    this.$('viz-section').classList.remove('hidden');
    this.$('viz-section').classList.add('fade-in');
  }

  _tick(t) {
    if (this.renderer) this.renderer.update(t);
    const d = this.engine.duration;
    this.$('scrubber').value = d > 0 ? (t / d) * 100 : 0;
    this.$('scrubber-current').textContent = Math.round(t) + ' ms';
    this._playBtn();
    if (this.player && this.player.playing) {
      const wrap = document.querySelector('.tl-scroll');
      if (wrap) {
        const il = 110 + t * this.renderer.pixelsPerMs;
        if (il > wrap.scrollLeft + wrap.clientWidth - 80 || il < wrap.scrollLeft + 40)
          wrap.scrollLeft = Math.max(0, il - 110);
      }
    }
  }

  _playBtn() {
    if (!this.player) return;
    this.$('btn-play').innerHTML = this.player.playing ? '\u23F8' : '\u25B6';
  }

  _stats(p) {
    const e = this.engine;
    this.$('stat-coders').textContent = e.numCoders;
    this.$('stat-dongles').textContent = e.numCoders;
    this.$('stat-events').textContent = e.events.length;
    this.$('stat-duration').textContent = e.duration + ' ms';
    this.$('stat-scheduler').textContent = p ? p.scheduler.toUpperCase() : '—';
    this.$('stat-completed').textContent = e.completedSet.size + '/' + e.numCoders;
    this.$('stat-burned').textContent = e.burnedSet.size + '/' + e.numCoders;
  }

  _validation(issues, parseErrors) {
    const sec = this.$('validation-section');
    sec.classList.remove('hidden');
    const errs = [...parseErrors, ...issues.filter(i => i.sev === 'error')];
    const warns = issues.filter(i => i.sev === 'warning');
    const infos = issues.filter(i => i.sev === 'info');

    this.$('val-summary').innerHTML = `
      <div class="val-stat pass"><div class="n">${this.engine.events.length}</div><div class="l">Events</div></div>
      <div class="val-stat err"><div class="n">${errs.length}</div><div class="l">Errors</div></div>
      <div class="val-stat warn"><div class="n">${warns.length}</div><div class="l">Warnings</div></div>
      <div class="val-stat info"><div class="n">${infos.length}</div><div class="l">Info</div></div>
    `;

    const det = this.$('val-details');
    det.innerHTML = '';
    const all = [...parseErrors.map(e => ({ ...e, sev: 'error' })), ...issues];
    if (all.length === 0) {
      det.innerHTML = '<div style="padding:1rem;color:var(--c-success);font-size:.8rem;text-align:center">\u2714 All validation checks passed</div>';
      return;
    }
    for (const iss of all) {
      const en = document.createElement('div');
      en.className = 'val-entry fade-in';
      en.dataset.line = iss.line || '';
      const sevChar = iss.sev === 'error' ? '\u2716' : iss.sev === 'warning' ? '\u26A0' : '\u2139';
      let meta = '';
      if (iss.line) meta += `<span class="val-chip">Line ${iss.line}</span>`;
      if (iss.coderId) meta += `<span class="val-chip">Coder ${iss.coderId}</span>`;
      if (iss.expected) meta += `<span class="val-chip exp">Expected: ${iss.expected}</span>`;
      if (iss.actual) meta += `<span class="val-chip act">Got: ${iss.actual}</span>`;
      en.innerHTML = `<div class="val-sev ${iss.sev}">${sevChar}</div><div class="val-body"><div class="val-msg">${iss.msg}</div><div class="val-detail">${iss.detail || ''}</div>${meta ? '<div class="val-meta">' + meta + '</div>' : ''}</div>`;
      en.addEventListener('click', () => {
        if (iss.line && this.engine) {
          const ev = this.engine.events.find(e => e.line === iss.line);
          if (ev && this.player) { this.player.pause(); this.player.seek(ev.time); }
        }
      });
      det.appendChild(en);
    }
  }

  _hoverErr(line, show) {
    const entries = this.$('val-details').querySelectorAll('.val-entry');
    for (const el of entries) {
      if (el.dataset.line === String(line)) {
        el.classList.toggle('highlight', show);
        if (show) el.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
      }
    }
  }

  _detail(cid) {
    if (!this.engine) return;
    const ci = this.engine.coderList.indexOf(cid);
    const color = CODER_COLORS[ci % CODER_COLORS.length];
    const p = this.params;
    const compiles = this.engine.compileCounts[cid];
    const burned = this.engine.burnedSet.has(cid);
    const done = this.engine.completedSet.has(cid);
    const ivs = this.engine.coderStates[cid] || [];
    let active = 0;
    for (const iv of ivs) if (['compiling', 'debugging', 'refactoring'].includes(iv.state)) active += iv.end - iv.start;
    const n = this.engine.numCoders;
    const i = cid - 1;
    const leftD = (i % 2 === 0) ? i + 1 : ((i + 1) % n) + 1;
    const rightD = (i % 2 === 0) ? ((i + 1) % n) + 1 : i + 1;
    const status = burned ? '\uD83D\uDD25 Burned' : done ? '\u2705 Done' : '\u23F3 Active';

    const modal = document.createElement('div');
    modal.className = 'card fade-in';
    modal.style.cssText = 'position:fixed;top:50%;left:50%;transform:translate(-50%,-50%);z-index:100;min-width:320px;max-width:90vw;box-shadow:0 8px 32px rgba(0,0,0,.6)';
    modal.innerHTML = `
      <div class="card-head"><h2 style="color:${color}">Coder ${cid}</h2><button class="btn btn-subtle" id="modal-close" style="font-size:1.1rem;padding:.2rem .5rem">&times;</button></div>
      <div style="display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:.6rem;padding:1rem">
        <div style="background:var(--bg-0);padding:.5rem .7rem;border-radius:var(--r-sm);border:1px solid var(--border)"><div style="font-size:.6rem;color:var(--fg-4);text-transform:uppercase">Status</div><div style="font-family:var(--mono);font-weight:700;font-size:.9rem;margin-top:.1rem">${status}</div></div>
        <div style="background:var(--bg-0);padding:.5rem .7rem;border-radius:var(--r-sm);border:1px solid var(--border)"><div style="font-size:.6rem;color:var(--fg-4);text-transform:uppercase">Compiles</div><div style="font-family:var(--mono);font-weight:700;font-size:.9rem;margin-top:.1rem">${compiles}${p ? '/' + p.compilesReq : ''}</div></div>
        <div style="background:var(--bg-0);padding:.5rem .7rem;border-radius:var(--r-sm);border:1px solid var(--border)"><div style="font-size:.6rem;color:var(--fg-4);text-transform:uppercase">Active Time</div><div style="font-family:var(--mono);font-weight:700;font-size:.9rem;margin-top:.1rem">${active}ms</div></div>
        <div style="background:var(--bg-0);padding:.5rem .7rem;border-radius:var(--r-sm);border:1px solid var(--border)"><div style="font-size:.6rem;color:var(--fg-4);text-transform:uppercase">Left Dongle</div><div style="font-family:var(--mono);font-weight:700;font-size:.9rem;margin-top:.1rem">D${leftD}</div></div>
        <div style="background:var(--bg-0);padding:.5rem .7rem;border-radius:var(--r-sm);border:1px solid var(--border)"><div style="font-size:.6rem;color:var(--fg-4);text-transform:uppercase">Right Dongle</div><div style="font-family:var(--mono);font-weight:700;font-size:.9rem;margin-top:.1rem">D${rightD}</div></div>
      </div>`;
    document.body.appendChild(modal);
    modal.querySelector('#modal-close').addEventListener('click', () => modal.remove());
    modal.addEventListener('click', (e) => { if (e.target === modal) modal.remove(); });
  }
}

/* ═══════════════════════════════════════════════════════════════
   EXAMPLE
   ═══════════════════════════════════════════════════════════════ */

const EXAMPLE_OUTPUT = `0 4 has taken a dongle
0 4 has taken a dongle
0 4 is compiling
0 2 has taken a dongle
0 2 has taken a dongle
0 2 is compiling
200 4 is debugging
200 2 is debugging
400 3 has taken a dongle
400 3 has taken a dongle
400 3 is compiling
400 1 has taken a dongle
400 1 has taken a dongle
400 1 is compiling
400 2 is refactoring
400 4 is refactoring
600 1 is debugging
600 3 is debugging
800 1 is refactoring
800 4 has taken a dongle
800 4 has taken a dongle
800 4 is compiling
800 2 has taken a dongle
800 2 has taken a dongle
800 2 is compiling
800 3 is refactoring
1000 2 is debugging
1000 4 is debugging
1200 1 has taken a dongle
1200 1 has taken a dongle
1200 1 is compiling
1200 3 has taken a dongle
1200 3 has taken a dongle
1200 3 is compiling
1200 4 is refactoring
1200 2 is refactoring
1400 3 is debugging
1400 1 is debugging
1600 2 has taken a dongle
1600 2 has taken a dongle
1600 2 is compiling
1600 4 has taken a dongle
1600 4 has taken a dongle
1600 4 is compiling
1600 1 is refactoring
1600 3 is refactoring
1800 2 is debugging
1800 4 is debugging
2000 1 has taken a dongle
2000 1 has taken a dongle
2000 1 is compiling
2000 4 is refactoring
2000 3 has taken a dongle
2000 3 has taken a dongle
2000 3 is compiling
2000 2 is refactoring
2200 3 is debugging
2200 1 is debugging
2400 4 has taken a dongle
2400 4 has taken a dongle
2400 4 is compiling
2400 3 is refactoring
2400 2 has taken a dongle
2400 2 has taken a dongle
2400 2 is compiling
2400 1 is refactoring
2600 4 is debugging
2600 2 is debugging
2800 1 has taken a dongle
2800 1 has taken a dongle
2800 1 is compiling
2800 3 has taken a dongle
2800 3 has taken a dongle
2800 3 is compiling`;

/* ═══════════════════════════════════════════════════════════════
   INIT
   ═══════════════════════════════════════════════════════════════ */

document.addEventListener('DOMContentLoaded', () => { new App(); });
