let project = new Project("plugins");

project.addFile("plugins.c");

if (platform === "windows") {
	project.addLib('proc_texsynth/win32/texsynth');
}
else if (platform === "linux") {
	project.addLib("texsynth -L" + project.basedir + "/proc_texsynth/linux");
}
else if (platform === "macos") {
	project.addLib(project.basedir + '/proc_texsynth/macos/libtexsynth.a');
}

return project;

