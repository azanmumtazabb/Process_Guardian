# Process Guardian

A Linux/Ubuntu terminal utility that lists active processes, monitors CPU/RAM use,
alerts on configurable limits, logs events, and controls selected processes.

## Build and run (Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential libncurses-dev
make
./process_guardian
```

## Controls

| Key | Action |
| --- | --- |
| Up / Down | Select a process |
| r | Refresh process data and evaluate alerts immediately |
| c | Set CPU alert threshold (percentage) |
| m | Set RAM alert threshold (MB) |
| p / s / t | Pause / resume / terminate selected process |
| n | Change selected process's nice value (-20 to 19) |
| q | Quit |

All starts, threshold changes, alerts, and process-control actions are appended to
`process_guardian.log` in the directory where the application is run.

Negative nice values require elevated permission. Run `sudo ./process_guardian` only
when you intentionally need to increase a process's priority.

The screen refreshes automatically every second. For a safe demonstration of the
pause, resume and terminate controls, start `sleep 300 &`, identify its PID with
`echo $!`, then select that PID in Process Guardian. Do not terminate important
system processes.

To compile the monitoring-only demonstration:

```bash
make monitor
./process_monitor
```
