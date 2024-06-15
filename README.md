# Aerials Demo

Have a try at the level ~~τo βe ίncluded~~ in `Aerials` :

#### 「+ERABY+E CONNEC+10N」

`Track:`  かめりあ

`Fumen:`  2001: memoir εμβαrκ

### Setup

1. Download the whole project.

2. Download `OPlusSans3-Regular.ttf` from [**ColorOS Official**](http://static01.coloros.com/www/public/img/topic7/font-opposans.zip) , and then put it into the `Reference` folder of the project.

3. Open `game.project` in the root of the project, with `Defold Editor` **v1.9.0 or upper**.

4. Use the menu choice `Project -> Bundle` to build the Demo.
   
   Currently this demo works with `Android / iOS / macOS / Windows` . *(You may need to put a valid `MSVCRT.lib` file into the `AcAudio/lib/x86_64-win32` directory to bundle a `Windows Application`)*

5. Install the bundle you built, and you should be ready to go. *(You need to **self-sign** the iOS bundle before installing it)*
   
   We'll also publish official releases aperiodically.

### Project Structure

```
Aerials Demo
 - AcArf3                        -- Engine Extension "Aerials Player v3"
      ···
 - AcAudio                       -- Engine Extension "AcAudio"
      ···
 - AcClipboard                   -- Engine Extension "defold-clipboard"
      ···
 - AcUtil                        -- Engine Extension "AcUtil"
      ···
 - Ar                            -- Game Object (*.go) files
      ···
 - Arf                           -- Aerials Chart[Fumen] (*.ar) files
      ···
 - Atlas                         -- Atlas (*.atlas) files
      ···
 - Pragma                        -- Common script files
      ···
 - Reference
    - Audio                      -- Audio files
         ···
    - Cover                      -- Blurred images
         ···
    - Illust                     -- Full-size images
         ···
    - Visual                     -- Visual elements images
         ···
    - Wish                       -- Images of each song's Wish
         ···
      lbase64.lua                -- Lua Module "lbase64"
      [OPlusSans3-Regular.ttf]   -- Put the Default Font File Here!
 - System                        -- Config & Platform-Specific files
      ···
   Ar.collection                 -- Bootstrap Collection
   Ar.license                    -- License file to be included
   Demo.script                   -- Logics of the Demo
   game.project                  -- Defold Game Project
```

### Complying with Licenses

- Refer to the `Ar.license` file.

- You may also copy the licenses into your clipboard:
  
  1. Bundle, install and open the demo.
  
  2. Tap the `Options ◎` button to expand the option panel.
  
  3. Tap the `Copy Credits To Clipboard` button.
