
// ugly workaround because Windows does weird things and ENOTIME
int real_main(int argc, char *argv[]);
int main(int argc, char *argv[]) { return real_main(argc, argv); }

#include <streamwindow.h>
#include <mainwindow.h>
#include <streamsession.h>
#include <settings.h>
#include <registdialog.h>
#include <host.h>
#include <avopenglwidget.h>
#include <controllermanager.h>

#ifdef CHIAKI_ENABLE_CLI
#include <chiaki-cli.h>
#endif

#include <chiaki/session.h>
#include <chiaki/regist.h>
#include <chiaki/base64.h>

#include <stdio.h>
#include <string.h>

#include <QApplication>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QCommandLineParser>
#include <QMap>
#include <QSurfaceFormat>
#include <QMessageBox>

Q_DECLARE_METATYPE(ChiakiLogLevel)

Settings settings;
DiscoveryManager discovery_manager;

void AlertAndQuit(const QString &message)
{
    QMessageBox::critical(nullptr, "Error", message, QMessageBox::Ok);
    QApplication::exit(1);
}

void FindHostAndStream()
{
    QList<DiscoveryHost> hosts = discovery_manager.GetHosts();
    if (hosts.isEmpty())
    {
        AlertAndQuit("No host found");
        return;
    }

    DiscoveryHost host = hosts.first();
    if (host.state != CHIAKI_DISCOVERY_HOST_STATE_READY)
    {
        AlertAndQuit("Console is not ready");
        return;
    }

    if (!settings.GetRegisteredHostRegistered(host.GetHostMAC()))
    {
        AlertAndQuit("No host registered");
        return;
    }

    RegisteredHost registered_host = settings.GetRegisteredHost(host.GetHostMAC());
    StreamSessionConnectInfo info(
            &settings,
            registered_host.GetTarget(),
            host.host_addr,
            registered_host.GetRPRegistKey(),
            registered_host.GetRPKey(),
            false,
            TransformMode::Fit);

    new StreamWindow(info);
}

int real_main(int argc, char *argv[])
{
	qRegisterMetaType<DiscoveryHost>();
	qRegisterMetaType<RegisteredHost>();
	qRegisterMetaType<HostMAC>();
	qRegisterMetaType<ChiakiQuitReason>();
	qRegisterMetaType<ChiakiRegistEventType>();
	qRegisterMetaType<ChiakiLogLevel>();

	QApplication::setOrganizationName("Chiaki");
	QApplication::setApplicationName("Chiaki");

	ChiakiErrorCode err = chiaki_lib_init();
	if(err != CHIAKI_ERR_SUCCESS)
	{
		fprintf(stderr, "Chiaki lib init failed: %s\n", chiaki_error_string(err));
		return 1;
	}

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QSurfaceFormat::setDefaultFormat(AVOpenGLWidget::CreateSurfaceFormat());

	QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/icons/chiaki.svg"));

    QObject::connect(&discovery_manager, &DiscoveryManager::HostsUpdated, &FindHostAndStream);
    QObject::connect(&settings, &Settings::RegisteredHostsUpdated, &FindHostAndStream);
    QObject::connect(&settings, &Settings::ManualHostsUpdated, &FindHostAndStream);

    settings.SetDiscoveryEnabled(true);
    discovery_manager.SetActive(true);
    return app.exec();
}
