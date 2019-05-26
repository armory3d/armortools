package ;
class Main {
    public static inline var projectName = 'ArmorPaint';
    public static inline var projectPackage = 'arm';
    public static inline var voxelgiVoxelSize = 2.0 / 256;
    public static inline var voxelgiHalfExtents = 1;
    public static function main() {
        iron.object.BoneAnimation.skinMaxBones = 50;
        armory.system.Starter.main(
            'Scene',
            0,
            true,
            true,
            true,
            1600,
            900,
            1,
            true,
            arm.renderpath.RenderPathCreator.get
        );
        new arm.App();
    }
}
