# Coil Array Block Project
This folder contains a KiCad hardware design for a magnetic-field sensing block.

## Project
### CoilArrayBlock
Analog sensing block containing only:

- Current transformer front end: CT-0705-HA
- Instrumentation amplifier stage: AD8422 (AD8422ARMZ)
- Gain-setting resistor network: R3 = 1.05k
- Output filter stage: R4 = 2k and C5 = 10nF
- Input/output connectors: JST B2B-XH-A (J1, J2, J3)

## Interfaces
- `Env_array_magfield`: magnetic field input to the sensing front end
- `Pwr_array_dcpwr`: `VCC` supply input
- `Pwr_array_vref`: `Vref` reference input
- `Array_sigproc_asig`: `Vout` analog signal output

## Repository Notes
This project includes KiCad source files (`.kicad_sch`, `.kicad_pcb`, `.kicad_pro`) and generated Gerbers.
Supporting documentation is included in `docs/`.
KiCad backup archives and local transient files are excluded via `.gitignore`.
