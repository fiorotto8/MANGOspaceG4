# MANGO in Space simulation

## GEANT

### Geometry

- For now just a 10cm side cube of gas surrounded by 1cm of Aluminum
- The entrance side has a 1.05cm diameter hole for entrance
- In the hole is supposed to be placed a MCP plate <https://tectra.de/wp-content/uploads/2017/07/MCP.pdf>
  - Geometry is implemented but commented since it crashed the graphics
- Getting primary energy of gamma and deposited energy in the gas, also other stuff but this is needed for efficiency
- Using `myMac.mac`

## ANALYSIS

- in analysis/ folder `efficiency.py`
- condition for *seen photon*: `(df["BoolInteracted"]==1) & (df["primaryHits"]==1 ) & (df["energyDeposit"]>df["primaryEnergy"]-epsilon)& (df["energyDeposit"]<=df["primaryEnergy"])`
  - espsilon usually 200eV
