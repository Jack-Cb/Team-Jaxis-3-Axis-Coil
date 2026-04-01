# Power Conditioning Block Projects

This repository contains two KiCad hardware designs for power conditioning and reference generation.

## Projects

### `PowerConditioningBlock`
A compact conditioning/reference stage centered on precision analog parts:

- `TPS7A94` low-noise LDO
- `ADR4525` precision voltage reference
- `OPA2211` precision op-amp
- Ferrite bead filtering and JST-style connectors for I/O

This folder is the streamlined core block for clean, low-noise regulated/reference power.

### `PowerBlock`
A more complete power subsystem that builds on the conditioning stage and adds front-end conversion/protection:

- `LM62460Q1` switching regulator stage with inductor
- `TPS7A94`, `ADR4525`, and `OPA2211` precision conditioning chain
- `TLV6710` comparator/supervisor function
- Input fuse, ferrite bead filtering, and multiple connector options

This folder represents the broader integrated power block with regulation, protection, and conditioning in one design.

## Repository Notes

- Each project includes KiCad source files (`.kicad_sch`, `.kicad_pcb`, `.kicad_pro`) and generated Gerbers.
- KiCad backup archives and local transient files are excluded via `.gitignore`.
- An unrelated local folder (`CoilArrayBlock/`) is intentionally not tracked in this repository.
