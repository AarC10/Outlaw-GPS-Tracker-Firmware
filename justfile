# Default recipe - show available commands
default:
    @just --list

# Build outlaw gen3 firmware into builds/outlaw
outlaw:
    west build -b outlaw_gen3 app -p auto --build-dir builds/outlaw

# Build deputy receiver firmware into builds/deputy
deputy:
    west build -b deputy receiver -p auto --build-dir builds/deputy

# Flash with ST-Link
# Usage: just sflash outlaw | just sflash deputy
sflash target:
    west flash --build-dir builds/{{target}}

# Flash with J-Link
# Usage: just jflash outlaw | just jflash deputy
jflash target:
    west flash --build-dir builds/{{target}} --runner=jlink
