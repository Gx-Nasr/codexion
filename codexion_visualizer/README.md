# Codexion Visualizer

Codexion Visualizer is a sleek browser-based tool for parsing, replaying, and validating multithreaded simulation output. It turns raw logs into an interactive timeline so you can inspect coder behavior, dongle ownership, timing rules, and validation issues in one place.

## Why this project exists

When simulation output gets long or hard to reason about, this visualizer makes it easier to:

- understand each coder’s lifecycle at a glance
- replay events with a simple playback interface
- spot timing and state-rule violations quickly
- compare expected behavior against generated logs

## Highlights

- Fast input flow with configuration + output log parsing
- Interactive timeline for each coder
- Live playback controls with pause, step, and speed adjustment
- Validation summaries for errors, warnings, and info
- Dongle state tracking and event log inspection
- Works without a build step: just open the app in your browser

## How it works

1. Enter the simulation configuration values.
2. Paste or upload the simulation output log.
3. Click Parse & Visualize.
4. Explore the timeline, validation results, and event history.

The app supports the following core parameters:

- coder count
- burnout threshold
- compile/debug/refactor durations
- required compiles
- cooldown
- scheduler mode: FIFO or EDF

## Interface overview

![Input and configuration view](readme_img/image1.png)

![Timeline replay and controls](readme_img/image2.png)

![Validation results and event log](readme_img/image3.png)

![Coder detail view](readme_img/image4.png)

## Features at a glance

- Playback controls: play, pause, step forward/backward, jump to start/end
- Keyboard shortcuts: Space to play/pause, arrow keys to step, Home/End to jump
- Per-coder detail inspection with compile counts and active time
- Real-time validation against the simulation rules

## Run locally

No installation is required. You can open [index.html](index.html) directly in a browser, or serve the folder with a simple local server:

```bash
python3 -m http.server 8000
```

Then visit http://localhost:8000/

## Project structure

- [index.html](index.html) — app shell and UI layout
- [styles.css](styles.css) — visual design and responsive layout
- [app.js](app.js) — parsing, validation, rendering, and playback logic
- [readme_img](readme_img) — screenshots used in this README

## License

This project is a lightweight visual tool for simulation debugging and analysis.
