ArmorPaint is currently an unsigned app. Due to security elements of macOS,
you will not be able to run the .app out of the box. Depending on the error message,
try one of the following.

1.) "ArmorPaint.app is damaged and can't be opened."
Open terminal in the unpacked ArmorPaint folder and type:
xattr -cr ArmorPaint.app

2.) "ArmorPaint can't be opened because it is from an unidentified developer."
Open terminal and type:
sudo spctl --master-disable

Afterwards macOS should let you open the ArmorPaint.app file.
