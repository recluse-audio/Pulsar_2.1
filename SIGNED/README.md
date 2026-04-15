# Signing Plugins #

This is platform dependent.

## Release Version ##
Change the update_version.py to count debug/release builds and version numbers separately. Debug Builds should display the debug build number and release the other.
They should keep track of these as separate variables. The plugin GUI will need to show the appropriate version based on build config.

## Test `SIGNED/PC/check_if_signed_pc.py` and `SIGNED/MAC/check_if_signed_mac.py` Script(s) ##
These scripts should be able to determine if a binary at a given path is signed or not.
It needs a crossplatform `SIGNED/test_digital_signature_check.py` to pc/mac version with paths to a known signed and a known unsigned binary.
It should use a vst3 in either case, taken from `../BUILD\Pulsar_artefacts\Release\VST3`


## Release Workflow Script ##
For this I want a single script that works crossplatform to do all the following. When ever possible it is preferred to use existing script architecture in this repo.

- i call from root directory `python HELPER_SCRIPTS/release_workflow.py`, this identifies pc/mac env and calls `SIGNED/release_workflow_pc.py` or the equivalent on mac.
- confirm release build(s) exist in expected location, confirm that installer project exists in expected location
- confirm release build(s) are not currently signed with `check_if_signed_pc.py`
- sign these release builds
- check these release builds are signed with `check_if_signed_pc.py` - or mac equivalent
- build an installer using the recently signed assets.
- installer gets signed, check digital signature with `check_if_signed_pc.py` or mac equivalent
- All signed assets (vst3/au/standalone)  as well as the newly signed installer is moved to a folder in RELEASE/PC or RELEASE/MAC depending on platform. 
- this folder in the RELEASE directory will be named according to RELEASE_VERSION