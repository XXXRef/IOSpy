SET regInitFilePath="iospy_reg_init.reg"
SET driverInstallFilePath="IOSpy.inf"

echo "IOSpy installer script"
echo ""

:: Preparing registry
echo "Injecting needed stuff into registry..."
%regInitFilePath%
echo "Done"

:: Install driver as service
echo "Installing driver..."
%driverInstallFilePath%
echo "Done"


