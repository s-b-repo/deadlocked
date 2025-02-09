# deadlocked

![downloads](https://img.shields.io/github/downloads/avitran0/deadlocked/total?color=blue)
[![foss cs2/deadlock hacking](https://badgen.net/discord/members/eXjG4Ar9Sx)](https://discord.gg/eXjG4Ar9Sx)

simple cs2 aimbot and esp, for linux only.

> [!CAUTION]
> vacnet 3.0 seems to be better at detecting aimbot and wallhacks, so **do not** use aim lock,
> and play with a low fov to avoid bans.
> use visuals sparingly, the default configuration should be a good starting point.

## features

- aimbot
  - fov
  - smoothing (with jitter)
  - aim lock
  - visibility check
  - head only/whole body
  - flash check
- esp
  - box
  - skeleton
  - health bar
  - armor bar
  - player name
  - weapon name
- rcs
- unsafe
  - noflash
    - max flash alpha
  - fov changer

> [!WARNING]
> the features in the unsafe tab are there for a reason.
> do not use them unless you are fine with risking a ban.
> they write to game memory.

## setup

- add your user to the `input` group: `sudo usermod -aG input USERNAME` (replace USERNAME with your actual username)
- restart your machine (this will **_not_** work without a restart!)
- clone the repository: `git clone --recursive https://github.com/avitran0/deadlocked`
- install cmake and a c++ compiler (should be preinstalled on most distros)

## running

- execute the run script: `./run.sh`
