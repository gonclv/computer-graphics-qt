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

    ObjetoGrafico(QVector<qreal> coordenadas, QPen pen) :
        coordenadas(coordenadas), pen(pen), cor(Qt::black) {}

    virtual void desenhar(QPainter& painter) = 0;

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

    void rotacionar(qreal angulo) {
        qreal radianos = qDegreesToRadians(angulo);
        int tamanhoMatriz = 4;

        //Primeiro precisamos transladar o ponto até o centro
        QPointF centroTriangulo = determinarCentro();
        transladar(QPointF(-(centroTriangulo.x()), -(centroTriangulo.y())));

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
        transladar(QPointF(centroTriangulo.x(), centroTriangulo.y()));
    }

    void escalar(qreal fatorX, qreal fatorY) {
        int tamanhoMatriz = 4;
        qreal fatorZ = 1;

        //Primeiro precisamos transladar o ponto ate o centro
        QPointF centroTriangulo = determinarCentro();
        transladar(QPointF(-(centroTriangulo.x()), -(centroTriangulo.y())));

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
        transladar(QPointF(centroTriangulo.x(), centroTriangulo.y()));
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

    int getContEscala() {
        return this->contEscala;
    }

    void deletar() {
        delete this;
    }

protected:
    bool selecionado = false;
    QPen pen;
    QColor cor = Qt::black;
    int contEscala = 0;

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
};

class Ponto : public ObjetoGrafico {
public:
    Ponto(QVector<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

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
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawPoint(coordenadas[0], coordenadas[1]);
        }
    }
};

class Reta : public ObjetoGrafico {
public:
    Reta(QList<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

    void desenhar(QPainter& painter) override {
        painter.setRenderHint(QPainter::Antialiasing);
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
    Triangulo(QList<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

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

        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPolygon(pointList.data(), pointList.size());
        pointList.clear();
    }
};

class Poligono : public ObjetoGrafico {
public:
    Poligono(QList<qreal> coordenadas, QPen pen) : ObjetoGrafico(coordenadas, pen) {}

    void desenhar(QPainter& painter) override {
        painter.setBrush(pen.color()); //Cor de preenchimento

        if (estaSelecionado()) {
            painter.setPen(QPen(Qt::red, 2)); //Borda vermelha
        } else {
            painter.setPen(QPen(cor, 2)); //Mudar para Qt::black para desenhar borda preta
        }

        if(coordenadas.size() <= 4) {
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawPoint(coordenadas[0], coordenadas[1]);
        }
        else {
            QVector<QPointF> pointList;

            for(int i = 0; i < coordenadas.size(); i += 4) {
                pointList.append(QPointF(coordenadas[i], coordenadas[i+1]));
            }

            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawPolygon(pointList.data(), pointList.size());
            pointList.clear();
        }
    }
};

class ButtonContainer : public QWidget {
public:
    //Botoes relacionados a criacao de objetos
    QPushButton *botaoReta, *botaoPonto, *botaoTriangulo, *botaoPoligono, *botaoDigitar;

    //Botoes relacionados as transformacoes
    QPushButton *botaoSelecionar, *botaoTransladar, *botaoRotacionar, *botaoEscalar;

    //Botao para mudanca de cor
    QPushButton *botaoCor;

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

        botaoPoligono = new QPushButton("Desenhar Polígono", this);
        botaoPoligono->move(10, 430);

        botaoSelecionar = new QPushButton("Selecionar Objeto", this);
        botaoSelecionar->move(10, 100);

        botaoCor = new QPushButton("Mudar Cor", this);
        botaoCor->move(10, 460);

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
    Canvas(QWidget *parent = nullptr) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);

        //Desenhar grama
        Poligono *q1 = new Poligono({-10000, 10000, 1, 1, -10000, 500, 1, 1, 10000, 500, 1, 1, 10000, 10000, 1, 1}, QPen(Qt::green, tamanhoCaneta));
        q1->desenhar(painter);

        int contObj = 0;
        for (ObjetoGrafico* objeto : displayFile) {
            objeto->desenhar(painter);

            if(objeto->getContEscala() % 200 == 0) {
                Poligono *p = new Poligono({(qreal) (rand() % this->width()), (qreal) (rand() % this->height()), 1, 1}, QPen(Qt::black, tamanhoCaneta));
                displayFile.push_back(p);
            }

            if(objeto->getContEscala() < 1000) {
                objeto->escalar(1.001, 1.001);
            }
            else {
                displayFile.remove(contObj);
                objeto->deletar();
            }
            contObj++;
        }
        update();
    }

    void mousePressEvent(QMouseEvent *event) override {
        //Obter coordenadas do clique do mouse
        QPointF clickPoint = event->pos();

        Poligono *p1 = new Poligono({(qreal) this->rect().center().x() - 1, (qreal) this->rect().center().y() - 1, 1, 1}, QPen(Qt::black, tamanhoCaneta));
        displayFile.push_back(p1);

        objetoSelecionado = nullptr;
        for (ObjetoGrafico *objeto : displayFile) {
            if (objeto->contemPonto(clickPoint)) {
                objeto->selecionar(true);
                objetoSelecionado = objeto;
            } else {
                objeto->selecionar(false);
            }
        }
        update();
    }

    void keyPressEvent(QKeyEvent *event) override {
        if(event->key() == Qt::Key_Escape) {
            close();
        }

        if(event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
            //Mover Esquerda
            for(ObjetoGrafico *objeto : displayFile) {
                int moveFactor = 10;
                for(int i = 0; i < objeto->coordenadas.size(); i += 4) {
                    objeto->coordenadas[i] -= (objeto->coordenadas[i] - this->width()) * 0.01;
                }
            }

            xminWindow -= 0.05;
            xmaxWindow -= 0.05;

            update();
        }

        if(event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
            //Centro da tela
            QPointF center = this->rect().center();

            //Mover Direita
            for(ObjetoGrafico *objeto : displayFile) {
                for(int i = 0; i < objeto->coordenadas.size(); i += 4) {
                    if(objeto->coordenadas[i] < center.x()) {
                        if(objeto->coordenadas[i+1] < center.y()) {
                            objeto->coordenadas[i+1] += (objeto->coordenadas[i] - center.x()) * 0.005;
                        }
                        else {
                            objeto->coordenadas[i+1] -= (objeto->coordenadas[i] - center.x()) * 0.005;
                        }
                    }
                    else {
                        if(objeto->coordenadas[i+1] < center.y()) {
                            objeto->coordenadas[i+1] -= (objeto->coordenadas[i] - center.x()) * 0.005;
                        }
                        else {
                            objeto->coordenadas[i+1] += (objeto->coordenadas[i] - center.x()) * 0.005;
                        }
                    }

                    objeto->coordenadas[i] -= 10;
                }
            }

            xminWindow += 0.05;
            xmaxWindow += 0.05;

            update();
        }
    }

public slots:
    void trocarCor() {
        if (selecionando && objetoSelecionado) {
            QColor newColor = QColorDialog::getColor(objetoSelecionado->getCor(), this, "Selecione uma Cor", QColorDialog::ShowAlphaChannel);

            if (newColor.isValid()) {
                objetoSelecionado->setCor(newColor);
                update();
            }
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

    }

    void moverDireita() {

    }

    void moverCima() {
        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 1; i < objeto->coordenadas.size(); i += 2) {
                objeto->coordenadas[i] += 150;
            }
        }

        yminWindow += 0.5;
        ymaxWindow += 0.5;

        update();
    }

    void moverBaixo() {
        for(ObjetoGrafico *objeto : displayFile) {
            for(int i = 1; i < objeto->coordenadas.size(); i += 2) {
                objeto->coordenadas[i] -= 150;
            }
        }

        yminWindow -= 0.5;
        ymaxWindow -= 0.5;

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

    QPointF pontoInicial, pontoFinal;
    int contadorCliques = 0, numPontos;

    bool desenharReta = false;
    bool desenharPonto = false;
    bool desenharTriangulo = false;
    bool desenharPoligono = false;

    bool selecionando = false;
    ObjetoGrafico* objetoSelecionado = nullptr;

    QVector<qreal> clickCoordinates;
    QList<ObjetoGrafico*> displayFile;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Canvas *window = new Canvas;
    window->setWindowTitle("Canvas - Qt");
    window->setStyleSheet("background-color: #2d91c2;");
    window->showFullScreen();

    return app.exec();
}
