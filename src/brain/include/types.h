/**
 * @file type.h
 * @brief Define as structs e enums usadas no projeto brain
 */
#pragma once

#include <string>
#include <vector>
#include <numeric>
#include <iterator>
#include <limits>
#include <rclcpp/rclcpp.hpp>

using namespace std;

/* ------------------ Struct ------------------------*/

// Dimensões do campo: medidas paralelas à linha de fundo são chamadas de comprimento e as perpendiculares de largura, independentemente do tamanho real das linhas
struct FieldDimensions
{
    double length = 0.0;            // comprimento do campo
    double width = 0.0;             // largura do campo
    double penaltyDist = 0.0;       // distância em linha reta da marca do pênalti à linha de fundo
    double goalWidth = 0.0;         // largura do gol
    double circleRadius = 0.0;      // raio do círculo central
    double penaltyAreaLength = 0.0; // comprimento da grande área
    double penaltyAreaWidth = 0.0;  // largura da grande área
    double goalAreaLength = 0.0;    // comprimento da pequena área
    double goalAreaWidth = 0.0;     // largura da pequena área
                              // Atenção: a grande área é maior que a pequena área; na prática, o "comprimento" delas é menor que a "largura". Essa nomenclatura segue as regras da competição.
};
// PinguimBots: campo 9 x 6 m (kid_size). NAO e o campo da CBR 2026, que e 14 x 9 m: a CBR usa FD_ADULTSIZE abaixo
// (field_type: adult_size no config_local.yaml).
const FieldDimensions FD_KIDSIZE{9, 6, 1.5, 2.6, 0.6, 0.8, 3.0, 0.8, 3.0};  // comprimento, largura, pênalti, gol, raio do círculo, grande área (comp., larg.), pequena área (comp., larg.)
// PinguimBots: campo da UFPel (mesmo valor do robocup_public). Para treinar na UFPel, troque pela linha abaixo.
// const FieldDimensions FD_KIDSIZE{4.38, 3.79, 1, 1.02, 0.375, 1.03, 2.03, 0.54, 1.03};
// Original da Booster na linha abaixo.
// const FieldDimensions FD_KIDSIZE{9, 6, 1.5, 2.6, 0.75, 2, 5, 1, 3};
// const FieldDimensions FD_ADULTSIZE{14, 9, 2.1, 2.6, 1.5, 3, 6, 1, 4};
const FieldDimensions FD_ADULTSIZE{14, 9, 2.1, 2.6, 1.5, 3, 6, 1, 4};  // CBR 2026 (14x9): comprimento, largura, pênalti, gol, raio do círculo, grande área (comp., larg.), pequena área (comp., larg.)
// const FieldDimensions FD_ROBOLEAGUE{22, 14, 3.6, 2.6, 2, 2.25, 6.9, 0.75, 3.9};
// const FieldDimensions FD_ROBOLEAGUE{22, 14, 3.5, 2.6, 2, 5, 8, 2, 5};
const FieldDimensions FD_ROBOLEAGUE{22.003, 14.126, 3.635, 2.6, 1.99, 5.221, 8.121, 2.307, 5.083};

// Pose2D: um ponto no plano e sua orientação
struct Pose2D
{
    double x = 0;
    double y = 0;
    double theta = 0; // rad, a partir do semieixo x positivo, positivo no sentido anti-horário
};

// Point: um ponto tridimensional
struct Point
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// Point2D: um ponto bidimensional
struct Point2D
{
    double x = 0.0;
    double y = 0.0;
};

// BoundingBox
struct BoundingBox
{
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;
};

// GameObject: armazena informações de entidades importantes da partida, como Ball, Goalpost etc. É mais completo que o detection::DetectedObject da mensagem /detect.
struct GameObject
{
    // --- obtidos da mensagem /detect ---
    string label;                // como o objeto foi classificado
    string color;                // cor do objeto. Só usada para Opponent, para distinguir aliados de adversários
    BoundingBox boundingBox;     // caixa de detecção do objeto na imagem da câmera; origem no canto superior esquerdo, x para a direita, y para baixo
    Point2D precisePixelPoint;   // posição precisa do objeto em pixels; só os pontos de referência do campo têm este dado
    double confidence = 0.0;     // confiança da detecção; para obstacle é um número maior que 0 que representa a quantidade de pontos acima do limiar na grade de obstáculos
    Point posToRobot;            // posição do objeto no sistema de coordenadas do corpo do robô; posição 2D, ignora z.

    // --- calculados na função processDetectedObject ---
    Point posToField;                                 // posição do objeto no sistema de coordenadas do campo; posição 2D, ignora z. x para frente, y para a esquerda.
    double range = 0.0;                               // distância em linha reta do objeto à projeção do centro do robô no plano do campo
    double pitchToRobot = 0.0, yawToRobot = 0.0;      // pitch e yaw do objeto em relação à frente do robô, em rad; positivos para baixo e para a esquerda
    rclcpp::Time timePoint;                           // instante em que o objeto foi detectado

    // --- obtidos no tratamento específico de cada tipo de objeto; só alguns objetos têm estes valores ---
    int id = -1;    // id identificado; -1 indica ainda não identificado
    string name;  // human readable id
    double idConfidence = 0.0; // confiança da identificação do id, no intervalo [0, 1],
    string info; // armazena informações extras

    // Posição prevista confiável quando a bola é perdida por pouco tempo; mantida apenas pelo preditor de trajetória da bola.
    Point reliablePredictPosToField;
    Point reliablePredictPosToRobot;
};

// Line: uma reta em qualquer sistema de coordenadas, representada por x0, y0, x1, y1.
struct Line {
    double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
};

struct FieldLineRef {
    std::string name;
    double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0; // os dois extremos, em metros
    bool isVertical = false;
};

// enum LineID {
//     LT, // left touch line
//     RT, // right touch line
//     M, // middle line
//     SB, // self bottom (goal line)
//     OB, // opposite bottom (goal line)
//     SGL, // self goal area left
//     SGH, // self goal area horizontal
//     SGR, // self goal area right
//     OGL, // opposite goal area left
//     OGH, // opposite goal area horizontal
//     OGR, // opposite goal area right
//     SPL, // self penalty area left
//     SPH, // self penalty area horizontal
//     SPR, // self penalty area right
//     OPL, // opposite penalty area left
//     OPH, // opposite penalty area horizontal
//     OPR // opoosite penalty area right
// };

enum class LineHalf {
    NA,
    Self,
    Opponent
};

enum class LineDir {
    NA,
    Horizontal,
    Vertical
};

enum class LineSide {
    NA,
    Left,
    Right
};

enum class LineType {
    NA,
    TouchLine,
    MiddleLine,
    GoalLine,
    GoalArea,
    PenaltyArea
};

// FieldLine armazena as informações de uma linha marcada no campo
struct FieldLine {
    Line posToField;
    Line posToRobot;
    Line posOnCam;
    rclcpp::Time timePoint;                           
    LineDir dir = LineDir::NA;
    LineHalf half = LineHalf::NA;
    LineSide side = LineSide::NA;
    LineType type = LineType::NA;
    double confidence = 0.0;
};

// Armazena a posição teórica de um ponto de referência do campo no mapa
struct MapMarking {
    double x = 0.0;
    double y = 0.0;
    string type; // TCross | LCross | XCross | PenaltyPoint
    string name; // name é uma string de quatro caracteres, ex.: LSCL. 1º: L|T|X|P indica o type; 2º: S|M|O indica o meio-campo; 3º: L|M|R indica esquerda/meio/direita; 4º: B(boundary)|P(penalty area)|G(gaol area)|C(circle) indica o retângulo (ou círculo) onde o ponto está
};

// VisionBox representa a região do campo de visão
struct VisionBox {
    vector<double> posToField;
    vector<double> posToRobot;
    rclcpp::Time timePoint;                           // instante em que o objeto foi detectado
};

struct TimestampedData {
    vector<double> data;
    rclcpp::Time timePoint;
};

// Levantar-se
struct RobotRecoveryStateData {
    uint8_t state = 0; // IS_READY = 0, IS_FALLING = 1, HAS_FALLEN = 2, IS_GETTING_UP = 3,
    uint8_t is_recovery_available = 0; // 1 for available, 0 for not available
    uint8_t current_planner_index = 0;
};

// Armazena a comunicação entre companheiros de equipe
struct TMStatus {
    string role = "not initialized"; // triker, goal_keeper
    bool isAlive = false; // se está em campo, não está cumprindo penalidade e a comunicação não foi perdida
    bool ballDetected = false;
    bool ballLocationKnown = false;
    double ballConfidence = 0.;
    double ballRange = 0.;
    double cost = 0.; // custo calculado do estado atual até conseguir chutar a bola
    bool isLead = true; // se está com a posse de bola
    bool isInVisualKick = false; // se está executando o chute visual, para evitar que vários robôs disputem a ação
    Point ballPosToField;
    Pose2D robotPoseToField;
    double kickDir = 0.; // direção prevista do chute
    double thetaRb = 0.; // ângulo do robô até a bola, sistema de coordenadas field
    int cmd = 0; // último comando enviado 
    int cmdId = 0; // ID do último comando enviado
    rclcpp::Time timeLastCom; // instante da última comunicação
};


enum class RobotRecoveryState {
    IS_READY = 0,
    IS_FALLING = 1,
    HAS_FALLEN = 2,
    IS_GETTING_UP = 3
};
