#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QInputDialog>
#include <QColorDialog>
#include <QGraphicsView>
#include <QLayout>
#include <QListWidget>

class ObjetoGrafico {
public:
    QList<qreal> coordenadas;

    ObjetoGrafico(QVector<qreal> coordenadas, QColor cor) :
        coordenadas(coordenadas), cor(cor), corCaneta(cor) {}

    virtual void desenhar(QPainter& painter) = 0;

    QPointF determinarCentro() {
        qreal centroX = 0, centroY = 0;

        for(int i = 0; i < coordenadas.size(); i += 4) {
            centroX += coordenadas[i];
            centroY += coordenadas[i+1];
        }

        centroX = centroX / (coordenadas.size()/4);
        centroY = centroY / (coordenadas.size()/4);
        return QPointF(centroX, centroY);
    }

    bool contemPonto(const QPointF& ponto) const {
        if(coordenadas.size() <= 4) {
            //Calcular a distância entre o ponto atual e o ponto do mouse
            qreal dx = ponto.x() - coordenadas[0];
            qreal dy = ponto.y() - coordenadas[1];
            qreal distance = sqrt(dx * dx + dy * dy);

            return distance < 10.0; //Aumentar o valor para aumentar sensibilidade
        }

        for (int i = 0; i < coordenadas.size(); i += 4) {
            qreal x1 = coordenadas[i];
            qreal y1 = coordenadas[i + 1];
            qreal x2 = coordenadas[(i + 4) % coordenadas.size()];
            qreal y2 = coordenadas[(i + 5) % coordenadas.size()];

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

    void transladar(const QPointF& delta) {
        int tamanhoMatriz = 4;
        qreal deltaZ = 0;

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
        matrizTranslacao[2][tamanhoMatriz - 1] = deltaZ;

        //Para cada ponto, realizar a multiplicacao de matrizes
        for(int i = 0; i < coordenadas.size(); i += 4) {
            //Salvando as coordenadas antigas
            qreal coordenadasOriginais[] = {coordenadas[i], coordenadas[i+1], coordenadas[i+2], coordenadas[i+3]};

            //Preparando as novas coordenadas
            coordenadas[i] = 0;
            coordenadas[i+1] = 0;
            coordenadas[i+2] = 0;
            coordenadas[i+3] = 0;

            for(int j = 0; j < tamanhoMatriz; j++) {
                for(int k = 0; k < tamanhoMatriz; k++) {
                    coordenadas[i+j] += matrizTranslacao[j][k] * coordenadasOriginais[k];
                }
            }
        }
    }

    void rotacionar(qreal angulo, QPointF centro) {
        qreal radianos = qDegreesToRadians(angulo);
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        transladar(QPointF(-(centro.x()), -(centro.y())));

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

        //Para cada ponto, realizar a multiplicacao de matrizes
        for(int i = 0; i < coordenadas.size(); i += 4) {
            //Salvando as coordenadas antigas
            qreal coordenadasOriginais[] = {coordenadas[i], coordenadas[i+1], coordenadas[i+2], coordenadas[i+3]};

            //Preparando as novas coordenadas
            coordenadas[i] = 0;
            coordenadas[i+1] = 0;
            coordenadas[i+2] = 0;
            coordenadas[i+3] = 0;

            for(int j = 0; j < tamanhoMatriz; j++) {
                for(int k = 0; k < tamanhoMatriz; k++) {
                    coordenadas[i+j] += matrizRotacao[j][k] * coordenadasOriginais[k];
                }
            }
        }

        //Voltando para a posição inicial
        transladar(QPointF(centro.x(), centro.y()));
    }

    void escalar(qreal fatorX, qreal fatorY, QPointF centro) {
        int tamanhoMatriz = 4;
        qreal fatorZ = 1;

        //Primeiro precisamos transladar o ponto ate o centro
        //QPointF centro = determinarCentro();
        transladar(QPointF(-(centro.x()), -(centro.y())));

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
        matrizEscala[2][2] = fatorZ;

        //Para cada ponto, realizar a multiplicacao de matrizes
        for(int i = 0; i < coordenadas.size(); i += 4) {
            //Salvando as coordenadas antigas
            qreal coordenadasOriginais[] = {coordenadas[i], coordenadas[i+1], coordenadas[i+2], coordenadas[i+3]};

            //Preparando as novas coordenadas
            coordenadas[i] = 0;
            coordenadas[i+1] = 0;
            coordenadas[i+2] = 0;
            coordenadas[i+3] = 0;

            for(int j = 0; j < tamanhoMatriz; j++) {
                for(int k = 0; k < tamanhoMatriz; k++) {
                    coordenadas[i+j] += matrizEscala[j][k] * coordenadasOriginais[k];
                }
            }
        }

        //Voltando para a posição inicial
        transladar(QPointF(centro.x(), centro.y()));
        contEscala++;
    }

    bool estaSelecionado() const {
        return selecionado;
    }

    void selecionar(bool selecionar) {
        selecionado = selecionar;
    }

    QColor getCor() {
        return this->cor;
    }

    void setCor(const QColor& cor) {
        this->cor = cor;
    }

    void setCorCaneta(const QColor& newColor) {
        this->corCaneta = newColor;
    }

    int getContEscala() {
        return this->contEscala;
    }

    void deletar() {
        delete this;
    }

protected:
    bool selecionado = false;
    QColor cor, corCaneta;
    int contEscala = 0;
    int tamanhoCaneta = 2;

    // Coordenadas
    qreal xmin = 0;
    qreal xmax = 600;
    qreal ymin = 0;
    qreal ymax = 600;

    int atribuiCode(int x1, int y1) {
        int code = 0;
        if(x1 > xmax) {
            code = 2; //Right
        }
        else if(x1 < xmin) {
            code = 1; //Left
        }

        if(y1 > ymax) {
            code = 4; //Bottom
        }
        else if(y1 < ymin) {
            code = 8; //Top
        }

        return code;
    }

    QVector<qreal> cohenSutherlandClip(int code, qreal x, qreal y) {
        QVector<qreal> newCoords = {x, y};
        qreal m = (coordenadas[3] - coordenadas[1]) / (coordenadas[2] - coordenadas[0]);

        if(code == 1) {
            newCoords[1] = m * (xmin - x) + y;
        }
        else if(code == 2) {
            newCoords[1] = m * (xmax - x) + y;
        }

        if(code == 4) {
            newCoords[0] = x + ((ymax - y) / m);
        }
        else if(code == 8) {
            newCoords[0] = x + ((ymin - y) / m);
        }

        return newCoords;
    }
};

class Ponto : public ObjetoGrafico {
public:
    Ponto(QVector<qreal> coordenadas, QColor cor) : ObjetoGrafico(coordenadas, cor) {}

    void desenhar(QPainter& painter) override {
        painter.setBrush(cor); //Cor de preenchimento

        if (estaSelecionado()) {
            painter.setPen(QPen(Qt::red, 2)); //Borda vermelha
        } else {
            painter.setPen(QPen(cor, 2)); //Mudar para Qt::black para desenhar borda preta
        }

        //Atribuir codigo para o clipping
        int code = atribuiCode(coordenadas[0], coordenadas[1]);

        //Clipping Cohen Sutherland
        if(code == 0) {
            painter.drawPoint(coordenadas[0], coordenadas[1]);
        }
    }
};

class Reta : public ObjetoGrafico {
public:
    Reta(QList<qreal> coordenadas, QColor cor) : ObjetoGrafico(coordenadas, cor) {}

    void desenhar(QPainter& painter) override {
        painter.setBrush(cor); //Cor de preenchimento

        if (estaSelecionado()) {
            painter.setPen(QPen(Qt::red, 2)); //Borda vermelha
        } else {
            painter.setPen(QPen(cor, 2)); //Mudar para Qt::black para desenhar borda preta
        }

        //Atribuir codigos para clipping
        int code1 = atribuiCode(coordenadas[0], coordenadas[1]);
        int code2 = atribuiCode(coordenadas[2], coordenadas[3]);

        //Clipping Cohen Sutherland
        if((code1 == 0) && (code2 == 0)) {
            painter.drawLine(coordenadas[0], coordenadas[1], coordenadas[2], coordenadas[3]);
        }
        else if((code1 && code2) == 0) {
            if(code1 != 0) {
                QVector<qreal> clipped = cohenSutherlandClip(code1, coordenadas[0], coordenadas[1]);
                painter.drawLine(clipped[0], clipped[1], coordenadas[2], coordenadas[3]);
            }
            else if(code2 != 0) {
                QVector<qreal> clipped = cohenSutherlandClip(code2, coordenadas[2], coordenadas[3]);
                painter.drawLine(coordenadas[0], coordenadas[1], clipped[0], clipped[1]);
            }
        }
    }
};

class Triangulo : public ObjetoGrafico {
public:
    Triangulo(QList<qreal> coordenadas, QColor cor) : ObjetoGrafico(coordenadas, cor) {}

    void desenhar(QPainter& painter) override {
        painter.setBrush(cor); //Cor de preenchimento

        if (estaSelecionado()) {
            painter.setPen(QPen(Qt::red, 2)); //Borda vermelha
        } else {
            painter.setPen(QPen(cor, 2)); //Mudar para Qt::black para desenhar borda preta
        }

        QVector<QPointF> pointList;

        for(int i = 0; i < coordenadas.size(); i += 4) {
            pointList.append(QPointF(coordenadas[i], coordenadas[i+1]));
        }

        painter.drawPolygon(pointList.data(), pointList.size());
        pointList.clear();
    }
};

class Poligono : public ObjetoGrafico {
public:
    Poligono(QList<qreal> coordenadas, QColor cor) : ObjetoGrafico(coordenadas, cor) {}
    Poligono(QList<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen.color()) {}

    void desenhar(QPainter& painter) override {
        painter.setBrush(cor); //Cor de preenchimento

        if (estaSelecionado()) {
            painter.setPen(QPen(Qt::red, 2)); //Borda vermelha
        } else {
            painter.setPen(QPen(corCaneta, 2)); //Mudar para Qt::black para desenhar borda preta
        }

        if(coordenadas.size() <= 4) {
            painter.drawPoint(coordenadas[0], coordenadas[1]);
        }
        else {
            QVector<QPointF> pointList;

            for(int i = 0; i < coordenadas.size(); i += 4) {
                pointList.append(QPointF(coordenadas[i], coordenadas[i+1]));
            }

            painter.drawPolygon(pointList.data(), pointList.size());
            pointList.clear();
        }
    }
};

class Cenario {
public:
    Cenario(qreal xmin, qreal xmax, qreal ymin, qreal ymax) {
        //Ceu
        poligonos.push_back(Poligono({xmin, ymin, 1, 1,
                                      xmax, ymin, 1, 1,
                                      xmax, ymax/2, 1, 1,
                                      xmin, ymax/2, 1, 1},
                                     Qt::blue));

        //Grama
        poligonos.push_back(Poligono({xmin, ymax/2, 1, 1,
                                      xmax, ymax/2, 1, 1,
                                      xmax, ymax, 1, 1,
                                      xmin, ymax, 1, 1},
                                     Qt::green));

        //Estrelas
        int numberOfStars = rand() % 90 + 10; //Desenha entre 10 e 99 estrelas
        for(int i = 0; i < numberOfStars; i++) {
            //Gerar coordenadas aleatorias para as estrelas
            qreal randomX = rand() % (int)xmax;
            qreal randomY = rand() % (int)ymax/2; //Metade da tela para cima
            poligonos.push_back(Poligono({randomX, randomY, 1, 1}, Qt::white));
        }

        //Detalhes da grama
        int numberOfDetails = rand() % 900 + 100; //Desenha entre 100 e 999 detalhes
        for(int i = 0; i < numberOfDetails; i++) {
            //Gerar coordenadas aleatorias para os detalhes
            qreal randomX = rand() % (int)xmax;
            qreal randomY = rand() % (int)ymax/2 + ymax/2; //metade da tela para baixo
            poligonos.push_back(Poligono({randomX, randomY, 1, 1}, QColor(0, 150, 0)));
        }

        //Cerca
        for(int x = xmin; x < xmax; x += 50) {
            Poligono p({(qreal)x, ymax/2, 1, 1,
                        (qreal)x, ymax/2 - 125, 1, 1,
                        (qreal)x + 25, ymax/2 - 150, 1, 1,
                        (qreal)x + 50, ymax/2 - 125, 1, 1,
                        (qreal)x + 50, ymax/2, 1, 1},
                        Qt::white);
            p.setCorCaneta(Qt::black);
            poligonos.push_back(p);
        }

        //Tronco da arvore
        poligonos.push_back(Poligono({300, 600, 1, 1,
                                      310, 400, 1, 1,
                                      290, 390, 1, 1,
                                      320, 390, 1, 1,
                                      330, 400, 1, 1,
                                      340, 390, 1, 1,
                                      360, 390, 1, 1,
                                      350, 400, 1, 1,
                                      340, 600, 1, 1},
                                     QColor(69,69,69)));

        //Folhas da árvore
        poligonos.push_back(Poligono({270, 390, 1, 1,
                                      230, 315, 1, 1,
                                      200, 300, 1, 1,
                                      160, 200, 1, 1,
                                      200, 150, 1, 1,
                                      220, 120, 1, 1,
                                      250, 120, 1, 1,
                                      350, 150, 1, 1,
                                      400, 220, 1, 1,
                                      450, 250, 1, 1,
                                      450, 290, 1, 1,
                                      440, 330, 1, 1,
                                      400, 350, 1, 1,
                                      390, 370, 1, 1,
                                      380, 380, 1, 1,
                                      370, 390, 1, 1},
                                     QColor(0,85,0)));

        //Arbusto
        poligonos.push_back(Poligono({500, 500, 1, 1,
                                      450, 475, 1, 1,
                                      455, 470, 1, 1,
                                      485, 450, 1, 1,
                                      505, 435, 1, 1,
                                      600, 435, 1, 1,
                                      610, 440, 1, 1,
                                      630, 460, 1, 1,
                                      625, 490, 1, 1,
                                      595, 500, 1, 1},
                                     QColor(0,85,0)));

        poligonos.push_back(
            Poligono({520, 470, 1, 1,
                                      560, 475, 1, 1,
                                      525, 455, 1, 1},
                                     QColor(0, 150, 0)));

        //Detalhes do arbusto
        poligonos.push_back(Poligono({600, 445, 1, 1,
                                      605, 470, 1, 1,
                                      585, 440, 1, 1},
                                     QColor(0, 150, 0)));
    }

    void desenhar(QPainter &painter) {
        for(Poligono p : poligonos) {
            p.desenhar(painter);
        }
    }

private:
    QList<Poligono> poligonos;
};

class Inimigo {
public:
    QList<Poligono *> poligonos;

    Inimigo() {
        QPen cor1 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord1 = {251.809, 237.746, 1, 1};
        poligono1 = new Poligono(coord1, cor1);
        poligonos.push_back(poligono1);

        QPen cor2 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord2 = {174.772, 91.7814, 1, 1, 308.573, 87.7268, 1, 1, 377.5, 189.091, 1, 1, 379.528, 304.646, 1, 1, 324.791, 371.547, 1, 1, 219.372, 371.547, 1, 1, 154.499, 308.701, 1, 1, 150.444, 174.9, 1, 1};
        poligono2 = new Poligono(coord2, cor2);
        poligonos.push_back(poligono2);

        QPen cor3 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord3 = { 255.863, 140.436, 1, 1, 217.345, 176.927, 1, 1, 219.372, 253.964, 1, 1, 261.945, 296.537, 1, 1, 300.463, 251.937, 1, 1, 298.436, 176.927, 1, 1};
        poligono3 = new Poligono(coord3, cor3);
        poligonos.push_back(poligono3);

        QPen cor4 (QColor::fromRgbF( 1, 1, 1),2);
        QList<qreal> coord4 = { 257.89, 172.873, 1, 1, 261.945, 258.019, 1, 1};
        poligono4 = new Poligono(coord4, cor4);
        poligonos.push_back(poligono4);

        QPen cor5 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord5 = { 227.481, 326.947, 1, 1, 302.491, 324.919, 1, 1};
        poligono5 = new Poligono(coord5, cor5);
        poligonos.push_back(poligono5);

        QPen cor6 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord6 = { 227.556, 326.421, 1, 1, 235.365, 341.438, 1, 1, 239.57, 325.82, 1, 1};
        poligono6 = new Poligono(coord6, cor6);
        poligonos.push_back(poligono6);

        QPen cor7 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord7 = { 291.829, 324.018, 1, 1, 299.037, 339.035, 1, 1, 302.641, 324.018, 1, 1};
        poligono7 = new Poligono(coord7, cor7);
        poligonos.push_back(poligono7);

        QPen cor8 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord8 = { 230.56, 326.421, 1, 1, 217.945, 313.206, 1, 1};
        poligono8 = new Poligono(coord8, cor8);
        poligonos.push_back(poligono8);

        QPen cor9 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord9 = {299.037, 325.82, 1, 1, 309.248, 314.408, 1, 1};
        poligono9 = new Poligono(coord9, cor9);
        poligonos.push_back(poligono9);

        QPen cor10 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord10 = {164.352, 124.66, 1, 1, 130.18, 80.9961, 1, 1, 159.605, 7.90625, 1, 1, 161.504, 80.0469, 1, 1, 170.047, 102.828, 1, 1};
        poligono10 = new Poligono(coord10, cor10);
        poligonos.push_back(poligono10);

        QPen cor11 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord11 = {320.973, 105.676, 1, 1, 334.262, 82.8945, 1, 1, 303.887, 5.05859, 1, 1, 375.078, 85.7422, 1, 1, 337.109, 128.457, 1, 1};
        poligono11 = new Poligono(coord11, cor11);
        poligonos.push_back(poligono11);

        QPen cor12 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord12 = {152.012, 252.805, 1, 1, 97.9063, 300.266, 1, 1};
        poligono12 = new Poligono(coord12, cor12);
        poligonos.push_back(poligono12);

        QPen cor13 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord13 = {96.957, 300.266, 1, 1, 129.23, 347.727, 1, 1};
        poligono13 = new Poligono(coord13, cor13);
        poligonos.push_back(poligono13);

        QPen cor14 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord14 = {129.125, 347.516, 1, 1, 149.375, 351.313, 1, 1, 131.234, 363.547, 1, 1};
        poligono14 = new Poligono(coord14, cor14);
        poligonos.push_back(poligono14);

        QPen cor15 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord15 = {131.656, 362.703, 1, 1, 142.625, 373.25, 1, 1, 138.828, 357.219, 1, 1};
        poligono15 = new Poligono(coord15, cor15);
        poligonos.push_back(poligono15);

        QPen cor16 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord16 = {138.406, 349.203, 1, 1, 144.313, 338.656, 1, 1, 144.734, 350.891, 1, 1};
        poligono16 = new Poligono(coord16, cor16);
        poligonos.push_back(poligono16);

        QPen cor17 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord17 = {141.359, 355.109, 1, 1, 150.641, 363.547, 1, 1, 146.422, 351.313, 1, 1};
        poligono17 = new Poligono(coord17, cor17);
        poligonos.push_back(poligono17);

        QPen cor18 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord18 = {378.5, 253.5, 1, 1, 416, 301.5, 1, 1};
        poligono18 = new Poligono(coord18, cor18);
        poligonos.push_back(poligono18);

        QPen cor19 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord19 = {415, 300.5, 1, 1, 384, 349.5, 1, 1};
        poligono19 = new Poligono(coord19, cor19);
        poligonos.push_back(poligono19);

        QPen cor20 (QColor::fromRgbF( 1, 0, 0),2);
        QList<qreal> coord20 = {383.5, 349, 1, 1, 369.5, 351, 1, 1, 385, 365.5, 1, 1};
        poligono20 = new Poligono(coord20, cor20);
        poligonos.push_back(poligono20);

        QPen cor21 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord21 = {369, 350.5, 1, 1, 365, 362, 1, 1, 374, 355, 1, 1};
        poligono21 = new Poligono(coord21, cor21);
        poligonos.push_back(poligono21);

        QPen cor22 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord22 = {378.5, 359, 1, 1, 372.5, 369.5, 1, 1, 384.5, 363, 1, 1};
        poligono22 = new Poligono(coord22, cor22);
        poligonos.push_back(poligono22);

        QPen cor23 (QColor::fromRgbF( 1, 0.666667, 0),2);
        QList<qreal> coord23 = {149, 63, 1, 1};
        poligono23 = new Poligono(coord23, cor23);
        poligonos.push_back(poligono23);

        //Mover inimigo
        int randomX = rand() % 1000;
        int randomY = rand() % 300 + 200; //metade da tela para baixo
        move(randomX, randomY);
    }

    void move(int a, int b){
        QPointF F(a,b);

        for(Poligono *p : poligonos) {
            p->transladar(F);
        }
    }

    void rotate(qreal angle) {
        for(Poligono *p : poligonos) {
            p->rotacionar(angle, poligono2->determinarCentro());
        }
    }
    void correr() {
        for(Poligono *p : poligonos) {
            p->escalar(1.001,1.001, poligono2->determinarCentro());
        }
    }

    void desenhar(QPainter &painter, int *contInimigos){
        if(inimigoSelecionado) {
            if(contMovimentacao < 500) {
                //Fazer o inimigo "morrer"
                rotate(90);
                contMovimentacao = 500;
            }

            if(contMovimentacao > 550) {
                //Inimigo morto
                *contInimigos = *contInimigos - 1;
                inimigoMorto = true;
                return; //Nao renderizar o inimigo
            }
        }
        else if(contMovimentacao < 500) {
            //Movimentar o inimigo
            correr();
        }

        //Desenhar o inimigo
        for(Poligono *p : poligonos) {
            p->desenhar(painter);
        }

        contMovimentacao++; //Indica que o inimigo se mexeu
    }

    void selectInimigo() {
        inimigoSelecionado = true;
    }

    bool getInimigoMorto() {
        return inimigoMorto;
    }

private:
    int contMovimentacao = 0;
    bool inimigoSelecionado = false, inimigoMorto = false;
    Poligono *poligono1, *poligono2, *poligono3, *poligono4, *poligono5,
        *poligono6, *poligono7, *poligono8, *poligono9, *poligono10,
        *poligono11, *poligono12, *poligono13, *poligono14, *poligono15,
        *poligono16, *poligono17, *poligono18, *poligono19, *poligono20,
        *poligono21, *poligono22, *poligono23;
};

class Canvas : public QWidget {
public:
    QList<ObjetoGrafico*> displayFile;

    Canvas(QWidget *parent = nullptr) : QWidget(parent) {
        btnCenario1 = new QPushButton("Cenário 1", this);
        btnCenario1->move(10, 10);
        connect(btnCenario1, &QPushButton::clicked, this, &Canvas::cenario1);
    }

protected:
    //Loop que desenha na tela
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        //Criar cenario se ja nao estiver criado
        if(c1 == nullptr) {
            xminViewport = 0;
            yminViewport = 0;
            xmaxViewport = this->width(); //Largura da janela
            ymaxViewport = this->height(); //Altura da janela

            c1 = new Cenario(xminViewport, xmaxViewport, yminViewport, ymaxViewport);
        }

        //Desenhar o cenario
        c1->desenhar(painter);

        if(contInimigos < dificuldade) {
            inimigos.push_back(new Inimigo());
            contInimigos++;
        }

        for(Inimigo *i : inimigos) {
            if(i->getInimigoMorto() == false) {
                i->desenhar(painter, &contInimigos);
            }
        }

        //A cada 1000 loops aumentar a dificuldade
        if(cont == 1000) {
            dificuldade++;
            cont = -1;
        }

        cont++;
        update();
    }

    void mousePressEvent(QMouseEvent *event) override {
        //Obter coordenadas do clique do mouse
        QPointF clickPoint = event->pos();

        //objetoSelecionado = nullptr;
        for(Inimigo *i : inimigos) {
            for (Poligono *p : i->poligonos) {
                if (p->contemPonto(clickPoint)) {
                    p->selecionar(true);
                    i->selectInimigo();
                    //objetoSelecionado = p;
                } else {
                    p->selecionar(false);
                }
            }
        }
    }

    void keyPressEvent(QKeyEvent *event) override {
        if(event->key() == Qt::Key_Escape) {
            close();
        }

        if(event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
            //Mover Esquerda
        }

        if(event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
            //Mover Direita
        }
    }

public slots:
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

    void cenario1() {
        desenharCenario1 = true;
        //desenharCenario2 = false;
        update();
    }

private:
    //Para uso ao desenhar cenarios
    Cenario *c1 = nullptr;
    QPushButton *btnCenario1;
    bool desenharCenario1 = false;

    //Desenhar inimigos
    int cont = 0, contInimigos = 0;
    QList<Inimigo *> inimigos;

    int dificuldade = 1;

    // Coordenadas viewport
    qreal xminViewport, xmaxViewport, yminViewport, ymaxViewport;

    // Coordenadas window
    qreal xminWindow = -1, xmaxWindow = 1;
    qreal yminWindow = -1, ymaxWindow = 1;

    QPointF pontoInicial, pontoFinal;
    int contadorCliques = 0, numPontos;

    bool selecionando = false;
    ObjetoGrafico* objetoSelecionado = nullptr;

    QVector<qreal> clickCoordinates;
};



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Canvas *window = new Canvas;
    window->setWindowTitle("Canvas - Qt");
    //window->setStyleSheet("background-color: #2d91c2;");
    window->showFullScreen();

    return app.exec();
}
