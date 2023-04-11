
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
#include <vpnclient.h>
#include <progressdialog.h>

#include <QApplication>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QCommandLineParser>
#include <QMap>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QThread>

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
    QList<ManualHost> manual_hosts = settings.GetManualHosts();
    if (manual_hosts.isEmpty())
    {
        AlertAndQuit("No host added");
        return;
    }

    ManualHost manual_host = manual_hosts.first();
    if (!settings.GetRegisteredHostRegistered(manual_host.GetMAC()))
    {
        AlertAndQuit("No host registered");
        return;
    }

    QObject::connect(&discovery_manager, &DiscoveryManager::HostsUpdated, [manual_host]() {
        DiscoveryHost discovery_host = discovery_manager.GetHosts().first();
        if (discovery_host.host_addr != manual_host.GetHost())
        {
            AlertAndQuit("No host discovered");
            return;
        }

        if (!manual_host.GetRegistered())
        {
            AlertAndQuit("Host no registered");
            return;
        }

        if (discovery_host.state != ChiakiDiscoveryHostState::CHIAKI_DISCOVERY_HOST_STATE_READY)
        {
            AlertAndQuit("Host is not ready");
            return;
        }

        RegisteredHost registered_host = settings.GetRegisteredHost(manual_host.GetMAC());
        StreamSessionConnectInfo info(
                &settings,
                registered_host.GetTarget(),
                manual_host.GetHost(),
                registered_host.GetRPRegistKey(),
                registered_host.GetRPKey(),
                false,
                TransformMode::Fit);

        new StreamWindow(info);
    });

    QString ip_address_str = manual_host.GetHost();
    discovery_manager.DiscoverDevice(ip_address_str);
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

    // VPN
    ProgressDialog progressDialog(app);
    OpenVPNClient client;
    progressDialog.show();

    QObject::connect(&client, &OpenVPNClient::logSignal, &progressDialog, &ProgressDialog::setMessage);
    if (true || client.init() == OpenVPNClient::CONNECTED) {
        progressDialog.accept();

        FindHostAndStream();
        //MainWindow main_window(&settings);
        //main_window.show();
        return app.exec();

    } else {
        progressDialog.reject();
        return 1;
    }
}
