# deadshotv1


### deadlocked fork

 abit more than simple cs2 aimbot and esp, for linux only.

## features

Randomizes process name (Linux only)

- aimbot
  - random vertical jitter
  - fov
  - smoothing (with jitter)
  - aim lock
  - visibility check
  - head only/whole body
  - flash check
  - fov circle

- esp
  - box
  - skeleton
  - health bar
  - armor bar
  - player name
  - weapon name
  - player tags (helmet, defuser, bomb)
  - dropped weapons

- triggerbot
  - random delays for stealth
  - min/max delay
  - hotkey
  - visibility check
  - flash check
  - scope check
- standalone rcs
- misc
  - sniper crosshair

-semi safe
 - fov changer only writes to memory when needed not the whole time
   
 - noflash
   
   random micro delay to evade heuristics

   small jitter for stealthy

   Clamp flash alpha to prevent illegal values

- unsafe
  
    - max flash alpha
  

> [!WARNING]
> the features in the unsafe tab are there for a reason.
> do not use them unless you are fine with risking a ban.
> they write to game memory.

> [!CAUTION]
> vacnet 3.0 seems to be better at detecting aimbot and wallhacks, so **you can** use aim lock just hide it,
> and play with a low fov to avoid bans. use visuals sparingly.

## setup

if possible, do not use the precompiled releases. they might be out of date.

- add your user to the `input` group: `sudo usermod -aG input $(whoami)`
- restart your machine (this will **_not_** work without a restart!)
- clone the repository: `git clone --recursive https://github.com/avitran0/deadlocked`
- install cmake and a c++17 compiler (gcc 7+ or clang 4+) 

## running

- execute the run script: `./run.sh`

## flags

- `--file-mem`: if process_vm_readv does not work/you get an error that it cannot get the resource offset
- `--scale=X`: override gui scale
- `--no-visuals`: disable visuals completely, if there are problems with the overlay
- `--verbose` or `-v`: increase log level by one
- `--silent`: decrease log level by one

## faq

### what desktop environments and window managers are supported?

it is tested on GNOME with Mutter, KDE with KWin, and SwayWM.
support for other (especially tiling) window managers is not guaranteed.
if in doubt, use either GNOME or KDE.

### the overlay window/my screen is black

your compositor or window manager does not support transparency, or it is not enabled.

### the overlay shows up, but i cannot click on anything

the window could not be made click-through, which might be because of window manager/compositor support.

### the overlay does not show up

you window manager does not support positioning or resizing the window.

### the overlay is not on top of other windows

your window manager does not support always on top windows.
