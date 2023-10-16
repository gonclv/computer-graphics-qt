#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QInputDialog>
#include <QGraphicsView>
#include <QLayout>
#include <QListWidget>

class ObjetoGrafico {
public:
    virtual ~ObjetoGrafico() {}

    virtual void desenhar(QPainter& painter) = 0;
    virtual bool contemPonto(const QPoint& ponto) const = 0;

    bool estaSelecionado() const {
        return selecionado;
    }

    void selecionar(bool selecionar) {
        selecionado = selecionar;
    }

    virtual void transladar(const QPointF& delta) = 0;
    virtual void rotacionar(float angulo) = 0;
    virtual void escalar(float fatorX, float fatorY) = 0;
    virtual void setCor(const QColor& cor) = 0;


protected:
    bool selecionado = false;
    float anguloRotacao = 45.0;
    float fatorEscalaX = 2.0;
    float fatorEscalaY = 2.0;
};

class Ponto : public ObjetoGrafico {
public:
    Ponto(QPointF ponto, QPen pen) : ponto(ponto), pen(pen) {}

    void desenhar(QPainter& painter) override {
        painter.setPen(pen);
        if (selecionado) {
            painter.setBrush(Qt::red);
            painter.setPen(QPen(Qt::red, 2)); // Contorno vermelho quando selecionado
        } else {
            painter.setBrush(cor);
        }
        painter.drawPoint(ponto.toPoint());
    }

    bool contemPonto(const QPoint& ponto) const override {
        // Calcule a distância entre o ponto atual e o ponto do mouse
        qreal dx = ponto.x() - this->ponto.x();
        qreal dy = ponto.y() - this->ponto.y();
        qreal distance = sqrt(dx * dx + dy * dy);

        // Defina um novo limite de seleção (por exemplo, 10 pixels)
        return distance < 10.0; // Aumente esse valor conforme necessário
    }

    void transladar(const QPointF& delta) override {
        int tamanhoMatriz = 4;

        qreal coordenadas[] = {ponto.x(), ponto.y(), 1, 1};
        qreal matrizTranslacao[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizTranslacao[i][j] = 1;
                }
                else {
                    matrizTranslacao[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da translacao
        matrizTranslacao[0][tamanhoMatriz - 1] = delta.x();
        matrizTranslacao[1][tamanhoMatriz - 1] = delta.y();
        //matrizTranslacao[2][tamanhoMatriz - 1] --> para coordenada z

        //Novas coordenadas
        qreal novasCoordenadas[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novasCoordenadas[i] += matrizTranslacao[i][k] * coordenadas[k];
            }
        }

        ponto.setX(novasCoordenadas[0]);
        ponto.setY(novasCoordenadas[1]);
    }

    void rotacionar(float angulo) override {
        QTransform transform;
        QPointF center =ponto;
        transform.translate(center.x(), center.y());
        transform.rotate(angulo);
        transform.translate(-center.x(), -center.y());
        ponto = transform.map(ponto);
    }
    void escalar(float fatorX, float fatorY) override {
        // Aumenta gradualmente o tamanho do ponto com base em um fator incremental
        static float fatorIncrementalX = 1.0;
        static float fatorIncrementalY = 1.0;
        fatorIncrementalX *= fatorX;
        fatorIncrementalY *= fatorY;

        // Aumenta ou diminui a largura da caneta proporcionalmente
        pen.setWidthF(pen.widthF() * fatorIncrementalX);
        pen.setWidthF(pen.widthF() * fatorIncrementalY);
    }



    void setCor(const QColor& cor) override {
        pen.setColor(cor);
    }




private:
    QPointF ponto;
    QPen pen;
    QColor cor = Qt::black;

};

class Reta : public ObjetoGrafico {
public:
    Reta(QLineF reta, QPen pen) : reta(reta), pen(pen) {}

    void desenhar(QPainter& painter) override {
        painter.setPen(pen);
        if (selecionado) {
            painter.setBrush(Qt::red);
            painter.setPen(QPen(Qt::red, 2)); // Contorno vermelho quando selecionado
        } else {
            painter.setBrush(cor);
        }
        painter.drawLine(reta);
    }

    bool contemPonto(const QPoint& ponto) const override {
        qreal x1 = reta.p1().x();
        qreal y1 = reta.p1().y();
        qreal x2 = reta.p2().x();
        qreal y2 = reta.p2().y();

        qreal A = ponto.x() - x1;
        qreal B = ponto.y() - y1;
        qreal C = x2 - x1;
        qreal D = y2 - y1;

        qreal dot = A * C + B * D;
        qreal len_sq = C * C + D * D;
        qreal param = dot / len_sq;

        qreal closestX, closestY;

        if (param < 0) {
            closestX = x1;
            closestY = y1;
        } else if (param > 1) {
            closestX = x2;
            closestY = y2;
        } else {
            closestX = x1 + param * C;
            closestY = y1 + param * D;
        }

        qreal dx = ponto.x() - closestX;
        qreal dy = ponto.y() - closestY;
        qreal distance = sqrt(dx * dx + dy * dy);

        return distance < 5.0; // Use o valor que considerar apropriado como limite
    }


    void transladar(const QPointF& delta) override {
        //reta.translate(delta); --> Funcao pronta
        int tamanhoMatriz = 4;

        qreal ponto1[] = {reta.x1(), reta.y1(), 1, 1};
        qreal ponto2[] = {reta.x2(), reta.y2(), 1, 1};
        qreal matrizTranslacao[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizTranslacao[i][j] = 1;
                }
                else {
                    matrizTranslacao[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da translacao
        matrizTranslacao[0][tamanhoMatriz - 1] = delta.x();
        matrizTranslacao[1][tamanhoMatriz - 1] = delta.y();
        //matrizTranslacao[2][tamanhoMatriz - 1] --> para coordenada z

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizTranslacao[i][k] * ponto1[k];
                novoPonto2[i] += matrizTranslacao[i][k] * ponto2[k];
            }
        }

        reta.setP1(QPointF(novoPonto1[0], novoPonto1[1]));
        reta.setP2(QPointF(novoPonto2[0], novoPonto2[1]));
    }

    void rotacionar(float angulo) override {
//        QTransform transform;
//        QPointF center = reta.center();
//        transform.translate(center.x(), center.y());
//        transform.rotate(angulo);
//        transform.translate(-center.x(), -center.y());
//        reta = transform.map(reta);
        //double radianos = angulo * M_PI /180.0;
        int tamanhoMatriz = 4;

        //Transladar para a origem
        QPointF centro = reta.center();
        const QPointF delta(-reta.center().x(), -reta.center().y());
        transladar(delta);

        qreal pontoOriginal1[] = {reta.x1(), reta.y1(), 1, 1};
        qreal pontoOriginal2[] = {reta.x2(), reta.y2(), 1, 1};

        qreal matrizRotacao[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizRotacao[i][j] = 1;
                }
                else {
                    matrizRotacao[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da rotacao
        //eixo x
        matrizRotacao[1][1] = cos(angulo);
        matrizRotacao[1][2] = -sin(angulo);
        matrizRotacao[2][1] = sin(angulo);
        matrizRotacao[2][2] = cos(angulo);

//        eixo y
//        matrizRotacao[0][0] = cos(radianos);
//        matrizRotacao[0][2] = sin(radianos);
//        matrizRotacao[2][0] = -sin(radianos);
//        matrizRotacao[2][2] = cos(radianos);

//        eixo z
//        matrizRotacao[0][0] = cos(radianos);
//        matrizRotacao[0][1] = -sin(radianos);
//        matrizRotacao[1][0] = sin(radianos);
//        matrizRotacao[1][1] = cos(radianos);

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizRotacao[i][k] * pontoOriginal1[k];
                novoPonto2[i] += matrizRotacao[i][k] * pontoOriginal2[k];
            }
        }

        reta.setP1(QPointF(novoPonto1[0], novoPonto1[1]));
        reta.setP2(QPointF(novoPonto2[0], novoPonto2[1]));

        //Retornar a posicao original
        transladar(centro);
    }

    void escalar(float fatorX, float fatorY) override {
        QPointF center = reta.center();
        QPointF newP1(center.x() - reta.dx() * fatorX / 2, center.y() - reta.dy() * fatorY / 2);
        QPointF newP2(center.x() + reta.dx() * fatorX / 2, center.y() + reta.dy() * fatorY / 2);

        reta.setP1(newP1);
        reta.setP2(newP2);
    }
    void setCor(const QColor& cor) override {
        pen.setColor(cor);
    }
private:
    QLineF reta;
    QPen pen;
    QColor cor = Qt::black;
};

class Triangulo : public ObjetoGrafico {
public:
    Triangulo(QPolygonF triangulo, QPen pen) : triangulo(triangulo), pen(pen) {}

    void desenhar(QPainter& painter) override {
        painter.setPen(pen);
        if (selecionado) {
            painter.setBrush(Qt::red);
            painter.setPen(QPen(Qt::red, 2));
        } else {
            painter.setBrush(cor);
        }
        painter.drawPolygon(triangulo);
    }

    bool contemPonto(const QPoint& ponto) const override {
        return triangulo.containsPoint(ponto, Qt::OddEvenFill);
    }

    void transladar(const QPointF& delta) override {
        //triangulo.translate(delta);
        int tamanhoMatriz = 4;

        qreal ponto1[] = {triangulo[0].x(), triangulo[0].y(), 1, 1};
        qreal ponto2[] = {triangulo[1].x(), triangulo[1].y(), 1, 1};
        qreal ponto3[] = {triangulo[2].x(), triangulo[2].y(), 1, 1};
        qreal matrizTranslacao[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizTranslacao[i][j] = 1;
                }
                else {
                    matrizTranslacao[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da translacao
        matrizTranslacao[0][tamanhoMatriz - 1] = delta.x();
        matrizTranslacao[1][tamanhoMatriz - 1] = delta.y();
        //matrizTranslacao[2][tamanhoMatriz - 1] --> para coordenada z

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};
        qreal novoPonto3[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizTranslacao[i][k] * ponto1[k];
                novoPonto2[i] += matrizTranslacao[i][k] * ponto2[k];
                novoPonto3[i] += matrizTranslacao[i][k] * ponto3[k];
            }
        }

        triangulo[0] = QPointF(novoPonto1[0], novoPonto1[1]);
        triangulo[1] = QPointF(novoPonto2[0], novoPonto2[1]);
        triangulo[2] = QPointF(novoPonto3[0], novoPonto3[1]);
    }

    void rotacionar(float angulo) override {
        QPointF center = triangulo.boundingRect().center();
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(angulo);
        transform.translate(-center.x(), -center.y());
        triangulo = transform.map(triangulo);
    }

    void escalar(float fatorX, float fatorY) override {
        QPointF center = triangulo.boundingRect().center();
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.scale(fatorX, fatorY);
        transform.translate(-center.x(), -center.y());
        triangulo = transform.map(triangulo);
    }

    void setCor(const QColor& cor) override {
        pen.setColor(cor);
    }

private:
    QPolygonF triangulo;
    QPen pen;
    QColor cor = Qt::black;
};

class ButtonContainer : public QWidget {
public:
    //Botoes relacionados a criacao de objetos
    QPushButton *botaoReta, *botaoPonto, *botaoTriangulo;

    //Botoes relacionados as transformacoes
    QPushButton *botaoSelecionar, *botaoTransladar, *botaoRotacionar, *botaoEscalar;

    //Construtor da classe ButtonContainer
    ButtonContainer(QWidget *parent = nullptr) : QWidget(parent) {
        botaoReta = new QPushButton("Desenhar Reta", this);
        botaoReta->move(10, 10);

        botaoPonto = new QPushButton("Desenhar Ponto", this);
        botaoPonto->move(10, 60);

        botaoTriangulo = new QPushButton("Desenhar Triângulo", this);
                         botaoTriangulo->move(10, 110);

        botaoSelecionar = new QPushButton("Selecionar", this);
        botaoSelecionar->move(10, 160);

        botaoTransladar = new QPushButton("Transladar", this);
        botaoTransladar->move(10, 210);

        botaoRotacionar = new QPushButton("Rotacionar", this);
        botaoRotacionar->move(10, 260);

        botaoEscalar = new QPushButton("Escalar", this);
        botaoEscalar->move(10, 310);
    }
};

class Canvas : public QWidget {
public:
    Canvas(ButtonContainer &botoes, QWidget *parent = nullptr) : QWidget(parent) {
        connect(botoes.botaoReta, &QPushButton::clicked, this, &Canvas::reta);
        connect(botoes.botaoPonto, &QPushButton::clicked, this, &Canvas::ponto);
        connect(botoes.botaoTriangulo, &QPushButton::clicked, this, &Canvas::triangulo);
        connect(botoes.botaoSelecionar, &QPushButton::clicked, this, &Canvas::selecionar);
        connect(botoes.botaoTransladar, &QPushButton::clicked, this, &Canvas::transladar);
        connect(botoes.botaoRotacionar, &QPushButton::clicked, this, &Canvas::rotacionar);
        connect(botoes.botaoEscalar, &QPushButton::clicked, this, &Canvas::escalar);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);

        for (ObjetoGrafico* objeto : displayFile) {
            objeto->desenhar(painter);
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (desenharReta) {
            if (contadorCliques == 0) {
                pontoInicial = event->pos();
                contadorCliques = 1;
            } else {
                pontoFinal = event->pos();
                QLineF linha(pontoInicial, pontoFinal);
                QPen pen(Qt::black);
                pen.setWidth(tamanhoCaneta);
                Reta* reta = new Reta(linha, pen);
                displayFile.push_back(reta);
                desenharReta = false;
                contadorCliques = 0;
                update();
            }
        }

        if (desenharPonto) {
            QPointF clickPoint = event->pos();
            Ponto* ponto = new Ponto(clickPoint, QPen(Qt::black, tamanhoCaneta));
            displayFile.push_back(ponto);
            desenharPonto = false;
            update();
        }

        if (desenharTriangulo) {
            trianguloAtual.append(event->pos());

            if (trianguloAtual.size() == 3) {
                QPolygonF triangulo = trianguloAtual;
                Triangulo* tri = new Triangulo(triangulo, QPen(Qt::black, tamanhoCaneta));
                displayFile.push_back(tri);
                trianguloAtual.clear();
                desenharTriangulo = false;
                update();
            }
        }

        if (selecionando) {
            objetoSelecionado = nullptr;
            for (ObjetoGrafico* objeto : displayFile) {
                if (objeto->contemPonto(event->pos())) {
                    objeto->selecionar(true);
                    objetoSelecionado = objeto;
                } else {
                    objeto->selecionar(false);
                }
            }
            update();
        }
    }

public slots:
    void reta() {
        desenharReta = !desenharReta;
        contadorCliques = 0;
    }
    void ponto() {
        desenharPonto = !desenharPonto;
    }
    void triangulo() {
        desenharTriangulo = !desenharTriangulo;
        if (!desenharTriangulo) {
            trianguloAtual.clear();
        }
    }

    void selecionar() {
        selecionando = !selecionando;
        if (!selecionando) {
            for (ObjetoGrafico* objeto : displayFile) {
                objeto->selecionar(false);
            }
            objetoSelecionado = nullptr;
            update();
        }
    }

    void transladar() {
        if (selecionando && objetoSelecionado) {
            bool ok;
            QString input = QInputDialog::getText(nullptr, "Digite as Coordenadas de Translação", "X, Y:", QLineEdit::Normal, "0, 0", &ok);

            if(ok) {
                QStringList parts = input.split(',');

                if (parts.size() == 2) {
                    qreal x = parts[0].trimmed().toDouble();
                    qreal y = parts[1].trimmed().toDouble();

                    objetoSelecionado->transladar(QPointF(x, y)); // Realiza a translação do objeto
                    update();
                }
            }
        }
    }

    void rotacionar() {
        if (selecionando && objetoSelecionado) {
            bool ok;
            QString input = QInputDialog::getText(nullptr, "Digite o Ângulo de Rotação", "Ângulo:", QLineEdit::Normal, "0.0", &ok);

            if (ok) {
                qreal rotationAngle = input.toDouble(); // Converte a entrada do usuário para um número de ponto flutuante
                objetoSelecionado->rotacionar(rotationAngle);
                update();
            }
        }
    }

    void escalar() {
        if (selecionando && objetoSelecionado) {
            bool ok;
            QString input = QInputDialog::getText(nullptr, "Digite os Fatores de Escala", "X, Y:", QLineEdit::Normal, "1.0, 1.0", &ok);

            if (ok) {
                QStringList parts = input.split(',');
                if (parts.size() == 2) {
                    qreal x = parts[0].trimmed().toDouble();
                    qreal y = parts[1].trimmed().toDouble();

                    objetoSelecionado->escalar(x, y); // Realiza a escala em torno do centro geométrico
                    update();
                }
            }
        }
    }


    // Função para mapear coordenadas normalizadas para a tela
    QPoint mapToScreen(const QPointF& point) {
        int x = static_cast<int>((point.x() - minX) / (maxX - minX) * width());
        int y = static_cast<int>((1.0 - point.y() - minY) / (maxY - minY) * height());
        return QPoint(x, y);
    }

private:
    int tamanhoCaneta = 2;

    // Coordenadas normalizadas
    qreal minX = -1.0;
    qreal maxX = 1.0;
    qreal minY = -1.0;
    qreal maxY = 1.0;

    bool desenharReta = false;
    QPointF pontoInicial, pontoFinal;
    int contadorCliques = 0;

    bool desenharPonto = false;
    bool desenharTriangulo = false;
    QPolygonF trianguloAtual;

    bool selecionando = false;
    ObjetoGrafico* objetoSelecionado = nullptr;

    std::vector<ObjetoGrafico*> displayFile;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *window = new QWidget;
    window->setWindowTitle("Canvas - Qt");
    //window->setStyleSheet("background-color: white;");
    window->resize(1200,600);
    //window->setGeometry(-1000, 1000, 200, 200);

    ButtonContainer *botoes = new ButtonContainer;
    botoes->setFixedWidth(150);

    Canvas *canvas = new Canvas(*botoes);
    canvas->setFixedWidth(1050);
    canvas->setStyleSheet("background-color: lightgrey;");
    //canvas->setMinimumSize(400, 400); // Defina o tamanho mínimo da janela

    //qreal minX = -1.0;
    //qreal maxX = 1.0;
    //qreal minY = -1.0;
    //qreal maxY = 1.0;

//    QAbstractItemModel model;

//    QStandardItemModel model;

//    QListView *list1 = new QListView;
//    list1->setModel(model);

    QGridLayout *layout = new QGridLayout(window);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(0);
    layout->addWidget(botoes, 0, 0);
    layout->addWidget(canvas, 0, 1);
    //layout->addWidget(list1, 0 ,2);

    window->show();

    return app.exec();
}
