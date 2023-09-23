Requirements: 
- Install Windows Subsystem for Linux (WSL)
- Build GopherCAN and Gopher Sense for a vehicle configuration
  - If you haven't done that before use this guide to install the needed packages https://docs.google.com/document/d/1cLWwNldL49tiXDKBKevhOdZic1N56Be6cRXwFWHZrO8/edit
  - Then, simply build a module in STM32 Cube IDE

1. Enter gdat_to_ld in WSL by navigating to it after opening WSL or by opening the location in command line and typing "wsl"
2. Run ./ld_converter [path to gdat file location]
3. Fill in the information on Venue, Session, etc.
4. Press the final enter and then the .ld file will be created with the same name in the same folder as the .gdat.

Commands to fix running batch.sh in WSL:
  awk '{ sub("\r$", ""); print }' batch.sh > batch2.sh
  mv batch2.sh batch.sh
