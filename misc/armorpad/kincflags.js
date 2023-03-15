
// Imported by armorcore/kfile.js
flags.name = 'ArmorPad';
flags.package = 'org.armorpad';
flags.with_nfd = true;
flags.with_tinydir = true;
flags.with_g2 = true;
flags.with_iron = true;
flags.on_project_created = function(project) {
	project.addDefine('IDLE_SLEEP');
}
