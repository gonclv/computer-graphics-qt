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
    QList<qreal> coordenadas;

    ObjetoGrafico(QVector<qreal> coordenadas, QPen pen) :
        coordenadas(coordenadas), pen(pen), cor(Qt::black) {}

    virtual void desenhar(QPainter& painter) = 0;
    virtual bool contemPonto(const QPoint& ponto) const = 0;
    virtual void transladar(const QPointF& delta) = 0;
    virtual void rotacionar(qreal angulo) = 0;
    virtual void escalar(qreal fatorX, qreal fatorY) = 0;
    virtual void setCor(const QColor& cor) = 0;

    bool estaSelecionado() const {
        return selecionado;
    }

    void selecionar(bool selecionar) {
        selecionado = selecionar;
    }

protected:
    bool selecionado = false;
    QPen pen;
    QColor cor = Qt::black;
};

class Ponto : public ObjetoGrafico {
public:
    Ponto(QVector<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

    void desenhar(QPainter& painter) override {
        painter.setPen(pen);
        if (estaSelecionado()) {
            painter.setBrush(Qt::red);
            painter.setPen(QPen(Qt::red, 2)); // Contorno vermelho quando selecionado
        } else {
            painter.setBrush(cor);
        }

        painter.drawPoint(coordenadas[0], coordenadas[1]);
    }

    bool contemPonto(const QPoint& ponto) const override {
        //Calcular a distância entre o ponto atual e o ponto do mouse
        qreal dx = ponto.x() - coordenadas[0];
        qreal dy = ponto.y() - coordenadas[1];
        qreal distance = sqrt(dx * dx + dy * dy);

        return distance < 10.0; //Aumentar o valor para aumentar sensibilidade
    }

    void transladar(const QPointF& delta) override {
        int tamanhoMatriz = 4;

        qreal coordenadasPonto[] = {coordenadas[0], coordenadas[1], 1, 1};
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
                novasCoordenadas[i] += matrizTranslacao[i][k] * coordenadasPonto[k];
            }
        }

        coordenadas[0] = novasCoordenadas[0];
        coordenadas[1] = novasCoordenadas[1];
    }

    void rotacionar(qreal angulo) override {
        qreal radianos = qDegreesToRadians(angulo);
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        qreal oldX = coordenadas[0];
        qreal oldY = coordenadas[1];
        transladar(QPointF(-coordenadas[0], -coordenadas[1]));

        qreal coordenadasPonto[] = {coordenadas[0], coordenadas[1], 1, 1};
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

        /*
        //Rotacao no eixo x
        matrizRotacao[1][1] = qCos(radianos);
        matrizRotacao[2][1] = qSin(radianos);
        matrizRotacao[2][2] = qCos(radianos);

        //Rotacao no eixo y
        matrizRotacao[0][0] = qCos(radianos);
        matrizRotacao[0][2] = qSin(radianos);
        matrizRotacao[2][0] = -(qSin(radianos));
        matrizRotacao[2][2] = qCos(radianos);
        */

        //Rotacao no eixo z
        matrizRotacao[0][0] = qCos(radianos);
        matrizRotacao[0][1] = -(qSin(radianos));
        matrizRotacao[1][0] = qSin(radianos);
        matrizRotacao[1][1] = qCos(radianos);

        //Novas coordenadas
        qreal novasCoordenadas[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novasCoordenadas[i] += matrizRotacao[i][k] * coordenadasPonto[k];
            }
        }

        coordenadas[0] = novasCoordenadas[0];
        coordenadas[1] = novasCoordenadas[1];

        //Voltando para a posição inicial
        transladar(QPointF(oldX, oldY));
    }

    void escalar(qreal fatorX, qreal fatorY) override {
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        qreal oldX = coordenadas[0];
        qreal oldY = coordenadas[1];
        transladar(QPointF(-coordenadas[0], -coordenadas[1]));

        qreal coordenadasPonto[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal matrizEscala[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizEscala[i][j] = 1;
                }
                else {
                    matrizEscala[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da translacao
        matrizEscala[0][0] = fatorX;
        matrizEscala[1][1] = fatorY;
        //matrizEscala[2][2] --> para coordenada z

        //Novas coordenadas
        qreal novasCoordenadas[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novasCoordenadas[i] += matrizEscala[i][k] * coordenadasPonto[k];
            }
        }

        coordenadas[0] = novasCoordenadas[0];
        coordenadas[1] = novasCoordenadas[1];

        //Voltando para a posição inicial
        transladar(QPointF(oldX, oldY));
    }

    void setCor(const QColor& cor) override {
        this->cor = cor;
        pen.setColor(cor);
    }
};

class Reta : public ObjetoGrafico {
public:
    Reta(QList<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

    void desenhar(QPainter& painter) override {
        painter.setPen(pen);
        if (estaSelecionado()) {
            painter.setBrush(Qt::red);
            painter.setPen(QPen(Qt::red, 2)); // Contorno vermelho quando selecionado
        } else {
            painter.setBrush(cor);
        }

        painter.drawLine(coordenadas[0], coordenadas[1], coordenadas[2], coordenadas[3]);
    }

    bool contemPonto(const QPoint& ponto) const override {
        qreal A = ponto.x() - coordenadas[0];
        qreal B = ponto.y() - coordenadas[1];
        qreal C = coordenadas[2] - coordenadas[0];
        qreal D = coordenadas[3] - coordenadas[1];

        qreal dot = A * C + B * D;
        qreal len_sq = C * C + D * D;
        qreal param = dot / len_sq;

        qreal closestX, closestY;

        if (param < 0) {
            closestX = coordenadas[0];
            closestY = coordenadas[1];
        } else if (param > 1) {
            closestX = coordenadas[2];
            closestY = coordenadas[3];
        } else {
            closestX = coordenadas[0] + param * C;
            closestY = coordenadas[1] + param * D;
        }

        qreal dx = ponto.x() - closestX;
        qreal dy = ponto.y() - closestY;
        qreal distance = sqrt(dx * dx + dy * dy);

        return distance < 10.0; // Aumentar a sensibilidade
    }

    void transladar(const QPointF& delta) override {
        int tamanhoMatriz = 4;

        qreal ponto1[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal ponto2[] = {coordenadas[2], coordenadas[3], 1, 1};
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

        coordenadas[0] = novoPonto1[0];
        coordenadas[1] = novoPonto1[1];
        coordenadas[2] = novoPonto2[0];
        coordenadas[3] = novoPonto2[1];
    }

    void rotacionar(qreal angulo) override {
        qreal radianos = qDegreesToRadians(angulo);
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        QPointF centroReta = determinarCentro();
        transladar(QPointF(-(centroReta.x()), -(centroReta.y())));

        qreal pontoOriginal1[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal pontoOriginal2[] = {coordenadas[2], coordenadas[3], 1, 1};

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
        /*
        //Rotacao no eixo x
        matrizRotacao[1][1] = qCos(radianos);
        matrizRotacao[2][1] = qSin(radianos);
        matrizRotacao[2][2] = qCos(radianos);

        //Rotacao no eixo y
        matrizRotacao[0][0] = qCos(radianos);
        matrizRotacao[0][2] = qSin(radianos);
        matrizRotacao[2][0] = -(qSin(radianos));
        matrizRotacao[2][2] = qCos(radianos);
        */

        //Rotacao no eixo z
        matrizRotacao[0][0] = qCos(radianos);
        matrizRotacao[0][1] = -(qSin(radianos));
        matrizRotacao[1][0] = qSin(radianos);
        matrizRotacao[1][1] = qCos(radianos);

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizRotacao[i][k] * pontoOriginal1[k];
                novoPonto2[i] += matrizRotacao[i][k] * pontoOriginal2[k];
            }
        }

        coordenadas[0] = novoPonto1[0];
        coordenadas[1] = novoPonto1[1];
        coordenadas[2] = novoPonto2[0];
        coordenadas[3] = novoPonto2[1];

        //Voltando para a posição inicial
        transladar(QPointF(centroReta.x(), centroReta.y()));
    }

    void escalar(qreal fatorX, qreal fatorY) override {
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        QPointF centroReta = determinarCentro();
        transladar(QPointF(-(centroReta.x()), -(centroReta.y())));

        qreal pontoOriginal1[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal pontoOriginal2[] = {coordenadas[2], coordenadas[3], 1, 1};
        qreal matrizEscala[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizEscala[i][j] = 1;
                }
                else {
                    matrizEscala[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da translacao
        matrizEscala[0][0] = fatorX;
        matrizEscala[1][1] = fatorY;
        //matrizEscala[2][2] --> para coordenada z

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizEscala[i][k] * pontoOriginal1[k];
                novoPonto2[i] += matrizEscala[i][k] * pontoOriginal2[k];
            }
        }

        coordenadas[0] = novoPonto1[0];
        coordenadas[1] = novoPonto1[1];
        coordenadas[2] = novoPonto2[0];
        coordenadas[3] = novoPonto2[1];

        //Voltando para a posição inicial
        transladar(QPointF(centroReta.x(), centroReta.y()));
    }

    void setCor(const QColor& cor) override {
        this->cor = cor;
        //pen.setColor(cor);
    }

private:
    QPointF determinarCentro() {
        qreal centroX = (coordenadas[0] + coordenadas[2])/2;
        qreal centroY = (coordenadas[1] + coordenadas[3])/2;
        return QPointF(centroX, centroY);
    }
};

class Triangulo : public ObjetoGrafico {
public:
    Triangulo(QList<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

    void desenhar(QPainter& painter) override {
        painter.setPen(pen);
        if (estaSelecionado()) {
            painter.setBrush(Qt::red);
            painter.setPen(QPen(Qt::red, 2));
        } else {
            painter.setBrush(cor);
        }

        QVector<QPointF> pointList;

        for(int i = 0; i < coordenadas.size(); i += 2) {
            pointList.append(QPointF(coordenadas[i], coordenadas[i+1]));
        }

        painter.drawPolygon(pointList.data(), pointList.size());
    }

    bool contemPonto(const QPoint& ponto) const override {
        for (int i = 0; i < coordenadas.size(); i += 2) {
            qreal x1 = coordenadas[i];
            qreal y1 = coordenadas[i + 1];
            qreal x2 = coordenadas[(i + 2) % coordenadas.size()];
            qreal y2 = coordenadas[(i + 3) % coordenadas.size()];

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

            if (distance < 10.0) {
                return true;
            }
        }

        return false;
    }

    void transladar(const QPointF& delta) override {
        int tamanhoMatriz = 4;

        qreal ponto1[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal ponto2[] = {coordenadas[2], coordenadas[3], 1, 1};
        qreal ponto3[] = {coordenadas[4], coordenadas[5], 1, 1};
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

        coordenadas[0] = novoPonto1[0];
        coordenadas[1] = novoPonto1[1];
        coordenadas[2] = novoPonto2[0];
        coordenadas[3] = novoPonto2[1];
        coordenadas[4] = novoPonto3[0];
        coordenadas[5] = novoPonto3[1];
    }

    void rotacionar(qreal angulo) override {
        qreal radianos = qDegreesToRadians(angulo);
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        QPointF centroTriangulo = determinarCentro();
        transladar(QPointF(-(centroTriangulo.x()), -(centroTriangulo.y())));

        qreal pontoOriginal1[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal pontoOriginal2[] = {coordenadas[2], coordenadas[3], 1, 1};
        qreal pontoOriginal3[] = {coordenadas[4], coordenadas[5], 1, 1};
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
        /*
        //Rotacao no eixo x
        matrizRotacao[1][1] = qCos(radianos);
        matrizRotacao[2][1] = qSin(radianos);
        matrizRotacao[2][2] = qCos(radianos);

        //Rotacao no eixo y
        matrizRotacao[0][0] = qCos(radianos);
        matrizRotacao[0][2] = qSin(radianos);
        matrizRotacao[2][0] = -(qSin(radianos));
        matrizRotacao[2][2] = qCos(radianos);
        */

        //Rotacao no eixo z
        matrizRotacao[0][0] = qCos(radianos);
        matrizRotacao[0][1] = -(qSin(radianos));
        matrizRotacao[1][0] = qSin(radianos);
        matrizRotacao[1][1] = qCos(radianos);

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};
        qreal novoPonto3[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizRotacao[i][k] * pontoOriginal1[k];
                novoPonto2[i] += matrizRotacao[i][k] * pontoOriginal2[k];
                novoPonto3[i] += matrizRotacao[i][k] * pontoOriginal3[k];
            }
        }

        coordenadas[0] = novoPonto1[0];
        coordenadas[1] = novoPonto1[1];
        coordenadas[2] = novoPonto2[0];
        coordenadas[3] = novoPonto2[1];
        coordenadas[4] = novoPonto3[0];
        coordenadas[5] = novoPonto3[1];

        //Voltando para a posição inicial
        transladar(QPointF(centroTriangulo.x(), centroTriangulo.y()));
    }

    void escalar(qreal fatorX, qreal fatorY) override {
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto ate o centro
        QPointF centroTriangulo = determinarCentro();
        transladar(QPointF(-(centroTriangulo.x()), -(centroTriangulo.y())));

        qreal pontoOriginal1[] = {coordenadas[0], coordenadas[1], 1, 1};
        qreal pontoOriginal2[] = {coordenadas[2], coordenadas[3], 1, 1};
        qreal pontoOriginal3[] = {coordenadas[4], coordenadas[5], 1, 1};
        qreal matrizEscala[tamanhoMatriz][tamanhoMatriz];

        //Criando matriz identidade
        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int j = 0; j < tamanhoMatriz; j++) {
                if(i == j) {
                    matrizEscala[i][j] = 1;
                }
                else {
                    matrizEscala[i][j] = 0;
                }
            }
        }

        //Preenchendo matriz com os dados da translacao
        matrizEscala[0][0] = fatorX;
        matrizEscala[1][1] = fatorY;
        //matrizEscala[2][2] --> para coordenada z

        //Novas coordenadas
        qreal novoPonto1[] = {0, 0, 0, 0};
        qreal novoPonto2[] = {0, 0, 0, 0};
        qreal novoPonto3[] = {0, 0, 0, 0};

        for(int i = 0; i < tamanhoMatriz; i++) {
            for(int k = 0; k < tamanhoMatriz; k++) {
                novoPonto1[i] += matrizEscala[i][k] * pontoOriginal1[k];
                novoPonto2[i] += matrizEscala[i][k] * pontoOriginal2[k];
                novoPonto3[i] += matrizEscala[i][k] * pontoOriginal3[k];
            }
        }

        coordenadas[0] = novoPonto1[0];
        coordenadas[1] = novoPonto1[1];
        coordenadas[2] = novoPonto2[0];
        coordenadas[3] = novoPonto2[1];
        coordenadas[4] = novoPonto3[0];
        coordenadas[5] = novoPonto3[1];

        //Voltando para a posição inicial
        transladar(QPointF(centroTriangulo.x(), centroTriangulo.y()));
    }

    void setCor(const QColor& cor) override {
        this->cor = cor;
    }

private:
    QPointF determinarCentro() {
        qreal centroX = (coordenadas[0] + coordenadas[2] + coordenadas[4])/3;
        qreal centroY = (coordenadas[1] + coordenadas[3] + coordenadas[5])/3;
        return QPointF(centroX, centroY);
    }
};

class ButtonContainer : public QWidget {
public:
    //Botoes relacionados a criacao de objetos
    QPushButton *botaoReta, *botaoPonto, *botaoTriangulo, *botaoDigitar;

    //Botoes relacionados as transformacoes
    QPushButton *botaoSelecionar, *botaoTransladar, *botaoRotacionar, *botaoEscalar;

    //Botoes relacionados a movimentacao de camera
    QPushButton *botaoMoverEsq, *botaoMoverDir, *botaoMoverCima, *botaoMoverBaixo, *botaoZoomIn, *botaoZoomOut;

    //Construtor da classe ButtonContainer
    ButtonContainer(QWidget *parent = nullptr) : QWidget(parent) {
        botaoReta = new QPushButton("Desenhar Reta", this);
        botaoReta->move(10, 10);

        botaoPonto = new QPushButton("Desenhar Ponto", this);
        botaoPonto->move(10, 40);

        botaoTriangulo = new QPushButton("Desenhar Triângulo", this);
        botaoTriangulo->move(10, 70);

        botaoSelecionar = new QPushButton("Selecionar", this);
        botaoSelecionar->move(10, 100);

        botaoTransladar = new QPushButton("Transladar", this);
        botaoTransladar->move(10, 130);

        botaoRotacionar = new QPushButton("Rotacionar", this);
        botaoRotacionar->move(10, 160);

        botaoEscalar = new QPushButton("Escalar", this);
        botaoEscalar->move(10, 190);

        botaoDigitar = new QPushButton("Digitar coordenadas", this);
        botaoDigitar->move(10, 220);

        botaoMoverEsq = new QPushButton("<", this);
        botaoMoverEsq->move(10, 250);

        botaoMoverDir = new QPushButton(">", this);
        botaoMoverDir->move(10, 280);

        botaoMoverCima = new QPushButton("^", this);
        botaoMoverCima->move(10, 310);

        botaoMoverBaixo = new QPushButton("v", this);
        botaoMoverBaixo->move(10, 340);

        botaoZoomIn = new QPushButton("+", this);
        botaoZoomIn->move(10, 370);

        botaoZoomOut = new QPushButton("-", this);
        botaoZoomOut->move(10, 400);
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
        connect(botoes.botaoDigitar, &QPushButton::clicked, this, &Canvas::digitarCoordenadas);
        connect(botoes.botaoMoverEsq, &QPushButton::clicked, this, &Canvas::moverEsquerda);
        connect(botoes.botaoMoverDir, &QPushButton::clicked, this, &Canvas::moverDireita);
        connect(botoes.botaoMoverCima, &QPushButton::clicked, this, &Canvas::moverCima);
        connect(botoes.botaoMoverBaixo, &QPushButton::clicked, this, &Canvas::moverBaixo);
        connect(botoes.botaoZoomIn, &QPushButton::clicked, this, &Canvas::zoomIn);
        connect(botoes.botaoZoomOut, &QPushButton::clicked, this, &Canvas::zoomOut);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);

        for (ObjetoGrafico* objeto : displayFile) {
            objeto->desenhar(painter);
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        //Obter coordenadas do clique do mouse
        QPointF clickPoint = event->pos();

        if (desenharPonto) {
            //Criar QVector com as coordenadas do ponto
            clickCoordinates.clear();
            clickCoordinates.push_back(clickPoint.x());
            clickCoordinates.push_back(clickPoint.y());

            //Criar o objeto ponto
            Ponto* ponto = new Ponto(clickCoordinates, QPen(Qt::black, tamanhoCaneta));

            //Inserir no displayFile
            displayFile.push_back(ponto);

            desenharPonto = false;
            update();
            return;
        }

        if (contadorCliques == 0) {
            clickCoordinates.clear();
        }

        //Criar QVector com as coordenadas do ponto
        clickCoordinates.push_back(clickPoint.x());
        clickCoordinates.push_back(clickPoint.y());
        contadorCliques++;

        if (desenharReta && (contadorCliques == 2)) {
            //Criar o objeto reta
            QPen pen(Qt::black);
            pen.setWidth(tamanhoCaneta);
            Reta *reta = new Reta(clickCoordinates, pen);

            //Inserir no displayFile
            displayFile.push_back(reta);

            desenharReta = false;
            contadorCliques = 0;
            update();
            return;
        }

        if (desenharTriangulo && (contadorCliques == 3)) {
            //Criar o objeto triangulo
            Triangulo *tri = new Triangulo(clickCoordinates, QPen(Qt::black, tamanhoCaneta));

            displayFile.push_back(tri);
            desenharTriangulo = false;
            contadorCliques = 0;
            update();
            return;
        }

        if (selecionando) {
            objetoSelecionado = nullptr;
            for (ObjetoGrafico *objeto : displayFile) {
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

    void digitarCoordenadas() {
        bool ok;
        QString input = QInputDialog::getText(nullptr, "Digite as coordenadas", "X, Y:", QLineEdit::Normal, "1.0, 1.0", &ok);

        if (ok) {
            QStringList parts = input.split(',');
            if (parts.size() == 2) {
                qreal xWindow = parts[0].trimmed().toDouble();
                qreal yWindow = parts[1].trimmed().toDouble();

                //Transformada de Viewport
                qreal xViewport = ((xWindow - xminWindow) / (xmaxWindow - xminWindow)) * (xmaxViewport - xminViewport);
                qreal yViewport = (1 - ((yWindow - yminWindow) / (ymaxWindow - yminWindow))) * (ymaxViewport - yminViewport);

                //Criar o objeto ponto
                Ponto* ponto = new Ponto({xViewport, yViewport}, QPen(Qt::black, tamanhoCaneta));

                //Inserir no displayFile
                displayFile.push_back(ponto);
                update();
            }
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

                    //Transformar delta em coordenadas window para viewport
                    qreal xViewport = x * (xmaxViewport/2); //x * 300
                    qreal yViewport = y * -(ymaxViewport/2); //y * -300

                    objetoSelecionado->transladar(QPointF(xViewport, yViewport)); // Realiza a translação do objeto
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

    void moverEsquerda() {
        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 0; i < objeto->coordenadas.size(); i += 2) {
                objeto->coordenadas[i] += 200;
            }
        }
        update();
    }

    void moverDireita() {
        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 0; i < objeto->coordenadas.size(); i += 2) {
                objeto->coordenadas[i] -= 200;
            }
        }
        update();
    }

    void moverCima() {
        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 1; i < objeto->coordenadas.size(); i += 2) {
                objeto->coordenadas[i] += 200;
            }
        }
        update();
    }

    void moverBaixo() {
        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 1; i < objeto->coordenadas.size(); i += 2) {
                objeto->coordenadas[i] -= 200;
            }
        }
        update();
    }

    void zoomIn() {
        QPointF center = this->rect().center();

        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 0; i < objeto->coordenadas.size(); i += 2) {
                //Zoom no eixo x
                objeto->coordenadas[i] += (objeto->coordenadas[i] - center.x()) * 0.5;

                //Zoom no eixo y
                objeto->coordenadas[i+1] += (objeto->coordenadas[i+1] - center.y()) * 0.5;
            }
        }
        update();
    }

    void zoomOut() {
        QPointF center = this->rect().center();

        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 0; i < objeto->coordenadas.size(); i += 2) {
                //Zoom no eixo x
                objeto->coordenadas[i] -= (objeto->coordenadas[i] - center.x()) * 0.5;

                //Zoom no eixo y
                objeto->coordenadas[i+1] -= (objeto->coordenadas[i+1] - center.y()) * 0.5;
            }
        }
        update();
    }

    /*
    // Função para mapear coordenadas normalizadas para a tela
    QPoint mapToScreen(const QPointF& point) {
        int x = static_cast<int>((point.x() - minX) / (maxX - minX) * width());
        int y = static_cast<int>((1.0 - point.y() - minY) / (maxY - minY) * height());
        return QPoint(x, y);
    }
    */

private:
    int tamanhoCaneta = 2;

    // Coordenadas viewport
    qreal xminViewport = 0, xmaxViewport = 600;
    qreal yminViewport = 0, ymaxViewport = 600;

    // Coordenadas window
    qreal xminWindow = -1, xmaxWindow = 1;
    qreal yminWindow = -1, ymaxWindow = 1;

    // Coordenadas normalizadas
    qreal minX = -1.0;
    qreal maxX = 1.0;
    qreal minY = -1.0;
    qreal maxY = 1.0;

    QPointF pontoInicial, pontoFinal;
    int contadorCliques = 0;

    bool desenharReta = false;
    bool desenharPonto = false;
    bool desenharTriangulo = false;

    bool selecionando = false;
    ObjetoGrafico* objetoSelecionado = nullptr;

    QVector<qreal> clickCoordinates;
    QPolygonF trianguloAtual;
    std::vector<ObjetoGrafico*> displayFile;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *window = new QWidget;
    window->setWindowTitle("Canvas - Qt");

    ButtonContainer *botoes = new ButtonContainer;
    botoes->setFixedWidth(150);
    botoes->setFixedHeight(600);

    Canvas *canvas = new Canvas(*botoes);
    canvas->setFixedWidth(600);
    canvas->setFixedHeight(600);
    canvas->setStyleSheet("background-color: lightgrey;");

    QGridLayout *layout = new QGridLayout(window);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(0);
    layout->addWidget(botoes, 0, 0);
    layout->addWidget(canvas, 0, 1);

    window->show();

    return app.exec();
}
