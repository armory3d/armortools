let project = new Project("plugins");

project.add_cfiles("plugins.c");

if (platform === "windows") {
	project.add_lib('proc_texsynth/win32/texsynth');
}
else if (platform === "linux") {
	project.add_lib("texsynth -L" + project.basedir + "/proc_texsynth/linux");
}
else if (platform === "macos") {
	project.add_lib(project.basedir + '/proc_texsynth/macos/libtexsynth.a');
}

return project;

