SET regInitFilePath=%CD%\iospy_reg_init.reg
SET driverInstallFilePath=%CD%\..\BUILD\IOSpy.inf

echo "IOSpy installer script"
echo ""

:: Preparing registry
echo "Injecting needed stuff into registry..."
regedit %regInitFilePath%
echo "Done"

:: Install driver as service
echo "Installing driver..."
%SystemRoot%\System32\InfDefaultInstall.exe %driverInstallFilePath%
echo "Done"


