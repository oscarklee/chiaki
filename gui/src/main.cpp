
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

void FindHostAndStream(ProgressDialog& progressDialog)
{
    QList<ManualHost> manual_hosts = settings.GetManualHosts();
    if (manual_hosts.isEmpty())
    {
        progressDialog.setMessage("No console added");
        return;
    }

    ManualHost manual_host = manual_hosts.first();
    if (!settings.GetRegisteredHostRegistered(manual_host.GetMAC()))
    {
        progressDialog.setMessage("No console registered");
        return;
    }

    if (!manual_host.GetRegistered())
    {
        progressDialog.setMessage("Manual console no registered");
        return;
    }

    QObject::connect(&discovery_manager, &DiscoveryManager::HostsUpdated, QApplication::instance(), [manual_host, &progressDialog]() {
        QList<DiscoveryHost> discovery_hosts = discovery_manager.GetHosts();
        if (discovery_hosts.empty()) {
            progressDialog.setMessage("Waking up...");
            return;
        }

        DiscoveryHost discovery_host = discovery_hosts.first();
        if (discovery_host.host_addr != manual_host.GetHost())
        {
            progressDialog.setMessage("No console discovered");
            return;
        }

        RegisteredHost registered_host = settings.GetRegisteredHost(discovery_host.GetHostMAC());
        if (discovery_host.state != ChiakiDiscoveryHostState::CHIAKI_DISCOVERY_HOST_STATE_READY)
        {
            progressDialog.setMessage("Console is not ready, sending wakeup command...");
            discovery_manager.SendWakeup(discovery_host.host_addr, registered_host.GetRPRegistKey(),
                                         chiaki_target_is_ps5(registered_host.GetTarget()));
            return;
        }

        StreamSessionConnectInfo info(
                &settings,
                registered_host.GetTarget(),
                discovery_host.host_addr,
                registered_host.GetRPRegistKey(),
                registered_host.GetRPKey(),
                false,
                TransformMode::Fit);

        progressDialog.accept();
        new StreamWindow(info);
    });

    QString ip_address_str = manual_host.GetHost();
    progressDialog.setMessage(QString("Looking for console '%1'...").arg(ip_address_str));
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

    QApplication::setOrganizationName("Rental");
    QApplication::setApplicationName("Rental");

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

    // Load settings
    settings.init();

    // VPN
    ProgressDialog progressDialog(app);
    OpenVPNClient client;
    progressDialog.show();

    //return app.exec();

    QObject::connect(&client, &OpenVPNClient::logSignal, &progressDialog, &ProgressDialog::setMessage);
    QObject::connect(&client, &OpenVPNClient::connected, QApplication::instance(), [&progressDialog]() {
        FindHostAndStream(progressDialog);
        //progressDialog.accept();
        //MainWindow main_window(&settings);
        //main_window.show();
    });

    client.start();
    return app.exec();
}
