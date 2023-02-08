.\dxc.exe -Zpr -Fo ..\raytrace_brute_core.cso -T lib_6_3 .\raytrace_brute.hlsl
.\dxc.exe -Zpr -Fo ..\raytrace_brute_full.cso -T lib_6_3 .\raytrace_brute.hlsl -D _FULL
.\dxc.exe -Zpr -Fo ..\raytrace_bake_ao.cso -T lib_6_3 .\raytrace_bake_ao.hlsl
.\dxc.exe -Zpr -Fo ..\raytrace_bake_light.cso -T lib_6_3 .\raytrace_bake_light.hlsl
.\dxc.exe -Zpr -Fo ..\raytrace_bake_bent.cso -T lib_6_3 .\raytrace_bake_bent.hlsl
.\dxc.exe -Zpr -Fo ..\raytrace_bake_thick.cso -T lib_6_3 .\raytrace_bake_thick.hlsl
