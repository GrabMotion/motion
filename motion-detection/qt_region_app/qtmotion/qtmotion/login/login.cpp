#include "login.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QNetworkAccessManager * networkManager;

using namespace std;

LoginDialog::LoginDialog (QWidget * parent) : QDialog (parent)
{
 setUpGUI();
 setWindowTitle( tr("User Login") );
 setModal( true );
}
 
void LoginDialog::setUpGUI()
{
     // set up the layout
     QGridLayout * formGridLayout = new QGridLayout( this );

     // initialize the SERVER URL combo box so that it is editable
     comboServerUrl = new QComboBox( this );
     comboServerUrl->setEditable( true );

     // initialize the username combo box so that it is editable
     comboUsername = new QComboBox( this );
     comboUsername->setEditable( true );
     // initialize the password field so that it does not echo
     // characters
     editPassword = new QLineEdit( this );
     editPassword->setEchoMode( QLineEdit::Password );     
     // initialize the client combo box so that it is editable
     comboClient = new QComboBox( this );
     comboClient->setEditable( true );
     comboClient->setEnabled( false );

     // initialize the labels
     labelServerUrl = new QLabel( this );
     labelUsername = new QLabel( this );
     labelPassword = new QLabel( this );
     labelClient = new QLabel( this );

     labelServerUrl->setText( tr( "Server" ) );
     labelServerUrl->setBuddy( comboServerUrl );
     labelUsername->setText( tr( "Username" ) );
     labelUsername->setBuddy( comboUsername );
     labelPassword->setText( tr( "Password" ) );
     labelPassword->setBuddy( editPassword );

     labelClient->setText( tr( "Client" ) );
     labelClient->setBuddy( labelClient );
     labelClient->setEnabled(false);

     // initialize buttons
     buttons = new QDialogButtonBox( this );
     buttons->addButton( QDialogButtonBox::Yes );
     buttons->addButton( QDialogButtonBox::Ok );
     buttons->addButton( QDialogButtonBox::Close );
     buttons->button( QDialogButtonBox::Yes )->setText( tr("Check") );
     buttons->button( QDialogButtonBox::Ok )->setText( tr("Login") );
     buttons->button( QDialogButtonBox::Close )->setText( tr("Close") );

     // connects slots
     connect( buttons->button( QDialogButtonBox::Yes ), SIGNAL (clicked()), this, SLOT (slotCheckAPI()) );
     connect( buttons->button( QDialogButtonBox::Ok ), SIGNAL (clicked()), this, SLOT (slotAcceptLogin()) );
     connect( buttons->button( QDialogButtonBox::Cancel ), SIGNAL (clicked()), this, SLOT (close()));

    // place components into the dialog
     formGridLayout->addWidget( labelServerUrl, 0, 0 );
     formGridLayout->addWidget( comboServerUrl, 0, 1 );

     formGridLayout->addWidget( labelUsername, 1, 0 );
     formGridLayout->addWidget( comboUsername, 1, 1 );

     formGridLayout->addWidget( labelPassword, 2, 0 );
     formGridLayout->addWidget( editPassword, 2, 1 );

     formGridLayout->addWidget( labelClient, 3, 0 );
     formGridLayout->addWidget( comboClient, 3, 1 );

     formGridLayout->addWidget( buttons, 4, 0, 1, 2 );
 
     setLayout( formGridLayout );

     QCoreApplication::setOrganizationName("DigiMotion Computer Vision");
     QCoreApplication::setOrganizationDomain("digimotion.com");
     QCoreApplication::setApplicationName("DigiMotion");
 
}
 
void LoginDialog::setUsername(QString &username)
{
     bool found = false;
     for( int i = 0; i < comboUsername->count() && ! found ; i++ )
     if( comboUsername->itemText( i ) == username )
     {
        comboUsername->setCurrentIndex( i );
        found = true;
     }

    if( ! found )
    {
        int index = comboUsername->count();
        qDebug() << "Select username " << index;
        comboUsername->addItem( username );
        comboUsername->setCurrentIndex( index );
    }

    // place the focus on the password field
     editPassword->setFocus();
}

void LoginDialog::setPassword(QString &password)
{
    editPassword->setText( password );
}

void LoginDialog::setServerUrl(QString &username)
{
     bool found = false;
     for( int i = 0; i < comboServerUrl->count() && ! found ; i++ )
     if( comboServerUrl->itemText( i ) == username )
     {
        comboServerUrl->setCurrentIndex( i );
        found = true;
     }

    if( ! found )
    {
        int index = comboServerUrl->count();
        qDebug() << "Select username " << index;
        comboServerUrl->addItem( username );
        comboServerUrl->setCurrentIndex( index );
    }

}

void LoginDialog::slotCheckAPI()
{
    QString url = comboServerUrl->currentText();
    std::stringstream link_api;
    link_api << "http://" << url.toStdString() << "/wp-json/wp/v2/client";
    std::stringstream aref;
    aref << "<a href='" << link_api.str() << "'>Client Ids</a>";
    std::string api_url = aref.str();

    //ui->api_link->setText(api_url.c_str());

    std::string linkapistd = link_api.str();
    std::cout << linkapistd << endl;
    QUrl urlapi(linkapistd.c_str());

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    networkManager->get(QNetworkRequest(QUrl(urlapi)));
}

void LoginDialog::replyFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 200)
    {
        QString strReply = (QString)reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
        QJsonArray jsonArray = jsonResponse.array();

        foreach (const QJsonValue & value, jsonArray) {
            QJsonObject clientobj = value.toObject();

            int id = clientobj["id"].toInt();
            QJsonObject title = clientobj["title"].toObject();
            QString clientname = title["rendered"].toString();

            int indexc = comboClient->findText(clientname);
            if (indexc==-1)
            {
               comboClient->addItem(clientname, QVariant(id));
            }
        }
     } else
    {
        QString reason = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
        cout << reason.toStdString() << endl;
    }
}

void LoginDialog::slotAcceptLogin()
{

    QString serverurl = comboServerUrl->currentText();
    QString username = comboUsername->currentText();
    QString password = editPassword->text();
    QString client = comboClient->currentText();
    int clientid = comboClient->itemData(comboClient->currentIndex()).toInt();

    emit acceptLogin(
        serverurl, // current server url       
        username, // current username
        password, // current password
        client,
        clientid // index in the username list
     );

    // close this dialog
     close();
}

void LoginDialog::setUsernamesList(const QStringList &usernames)
{
    comboUsername->addItems( usernames );
}

void LoginDialog::setClientsList(const QStringList &clients)
{
    comboUsername->addItems( clients );
}

