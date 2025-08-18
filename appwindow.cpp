// AppWindow.cpp
#include "appwindow.h"
#include "panelusuario.h"
#include "utils.h"
#include "Artist.h"
#include "panelartista.h"

#include <QVBoxLayout>

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
{
    // Fija la ventana a 1200×800
    setFixedSize(1200, 800);
    centerOnScreen(this);

    // 1) Crear el QStackedWidget
    stack = new QStackedWidget(this);
    stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack->setContentsMargins(0, 0, 0, 0);

    // 2) Helper para añadir páginas
    auto addPage = [&](QWidget *page) {
        page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        page->setContentsMargins(0, 0, 0, 0);
        stack->addWidget(page);
    };

    // 3) Instanciar páginas
    loginPage    = new Login(this);
    registroPage = new Registro(this);
    userPage1    = new UsuarioNormal(this);
    userPage2    = new UsuarioNormal2(this);
    artistPage1  = new RegistroArtista(this);
    artistPage2  = new RegistroArtista2(this);

    // 4) Añadirlas al stack
    addPage(loginPage);
    addPage(registroPage);
    addPage(userPage1);
    addPage(userPage2);
    addPage(artistPage1);
    addPage(artistPage2);

    // 5) Layout sin márgenes ni espaciado
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(stack);
    setLayout(lay);

    // 6) Conectar señales de navegación
    connect(loginPage,    &Login::goToRegistro,     this, &AppWindow::onShowRegistro);
    connect(loginPage,  &Login::goToUserPanel,
            this,       &AppWindow::onShowUserPanel);
    connect(loginPage,  &Login::goToArtistPanel,
            this,       &AppWindow::onShowArtistPanel);
    connect(registroPage, &Registro::goToLogin,     this, &AppWindow::onShowLogin);
    connect(registroPage, &Registro::goToUserReg,   this, &AppWindow::onShowUser1);
    connect(registroPage, &Registro::goToArtistReg, this, &AppWindow::onShowArtist1);

    connect(userPage1,    &UsuarioNormal::goToRegistro, this, &AppWindow::onShowRegistro);
    connect(userPage1,    &UsuarioNormal::goToUser2,this, &AppWindow::onShowUser2);

    connect(userPage1, &UsuarioNormal::datosUsuarioCapturados,
            userPage2, &UsuarioNormal2::setDatosUsuario);

    connect(userPage2, &UsuarioNormal2::goToUser1,   this, &AppWindow::onShowUser1);
    connect(userPage2, &UsuarioNormal2::goToUserPanel,
            this,       &AppWindow::onShowUserPanel);
    connect(artistPage1, &RegistroArtista::goToRegistro, this, &AppWindow::onShowRegistro);
    connect(artistPage1,  &RegistroArtista::goToArtist2,this, &AppWindow::onShowArtist2);

    connect(artistPage1,  &RegistroArtista::datosArtistaCapturados,
            artistPage2,  &RegistroArtista2::setDatosArtista);

    connect(artistPage2, &RegistroArtista2::goToArtist1, this, &AppWindow::onShowArtist1);
    connect(artistPage2, &RegistroArtista2::goToArtistPanel,
            this,         &AppWindow::onShowArtistPanel);


    // 7) Página inicial
    stack->setCurrentIndex(0);
}

// ——— Definición de los slots ———

//void AppWindow::onShowLogin()    { stack->setCurrentIndex(0); }
void AppWindow::onShowRegistro() { stack->setCurrentIndex(1); }
void AppWindow::onShowUser1()    { stack->setCurrentIndex(2); }
void AppWindow::onShowUser2()    { stack->setCurrentIndex(3); }
void AppWindow::onShowArtist1()  { stack->setCurrentIndex(4); }
void AppWindow::onShowArtist2()  { stack->setCurrentIndex(5); }

void AppWindow::onShowUserPanel(const User &user) {
    if (panelUsuario) {
        stack->removeWidget(panelUsuario);
        delete panelUsuario;
    }
    panelUsuario = new PanelUsuario(user, this);
    panelUsuario->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    panelUsuario->setContentsMargins(0,0,0,0);
    // Conectar cerrar sesión → volver a login
    connect(panelUsuario, &PanelUsuario::goToLogin, this,  &AppWindow::onShowLogin);
    stack->addWidget(panelUsuario);
    stack->setCurrentWidget(panelUsuario);
}

void AppWindow::onShowLogin() {
    loginPage->clearFields();
    stack->setCurrentIndex(0);
}

void AppWindow::onShowArtistPanel(const Artist &artist) {
    // Elimina panel anterior si existiera
    if (panelArtista) {
        stack->removeWidget(panelArtista);
        delete panelArtista;
    }
    // Crea nuevo panel
    panelArtista = new PanelArtista(artist, this);
    panelArtista->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack->addWidget(panelArtista);
    stack->setCurrentWidget(panelArtista);

    // Conectar “Cerrar sesión” (o volver) si tu PanelArtista emite esa señal
    connect(panelArtista, &PanelArtista::goToLogin,
            this,           &AppWindow::onShowLogin);
}





