from pathlib import Path

Import("env")

framework_dir = Path(env.PioPlatform().get_package_dir("framework-arduinoespressif32"))

for relative_path in ("libraries/FS/src", "libraries/SPIFFS/src"):
    include_dir = framework_dir / relative_path
    if include_dir.exists():
        env.Append(CPPPATH=[str(include_dir)])
