#import "motion.h"

@implementation Motion
/*
Motion* the = [[Motion alloc]init];
+ (Motion*) the {return the;};

-(void)configureAccelerometerWithGyroHandler: (Kt::GyroHandler*) gyroHandler
                           AndUpdateInterval: (int) updateInterval
{
    UIAccelerometer *accelerometer = [UIAccelerometer sharedAccelerometer];
    accelerometer.updateInterval = 1 / updateInterval;

    accelerometer.delegate = self;
    self->gyroHandler = gyroHandler;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration{
    //gyroHandler->updateAccelerometerData(acceleration.x, acceleration.y, acceleration.z);
}

void Kt::GyroHandler::configAccelerometer(int updateInterval){
    [[Motion the]configureAccelerometerWithGyroHandler:this AndUpdateInterval:updateInterval];
};
*/
@end
