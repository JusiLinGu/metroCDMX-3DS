/*Incluir las librerias principales de la 3DS como graficos, botones, audio, etc.*/
#include <3ds.h>
/*Para usar printf en mis primeras pruebas*/
#include <stdio.h>
/*Para permitir cargar imagenes*/
#include <citro2d.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    /*Inicia el sistema grafico y configura las pantallas con valores default*/
    gfxInitDefault();
    
    /*Inicializa la GPU de 3DS, (...) tamaño estandar de buffer para enviar comandos a la GPU*/
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    
    /*Inicializar citro2d que es la libreria que facilita dibujar sprites, imagenes, formas y texto en 2d*/
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    
    /*Se prepara para dibujar en la pantalla superior (o en la que se indique)
    Se llama despues de C2D_INIT y antes del loop*/
    C2D_Prepare();

    C3D_RenderTarget* top;
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    
    /*Inicializar romfs para cargar texturas*/
    romfsInit();

    /*Cargar sprite sheet*/
    C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/mapaMetroCDMX.t3x");
    if (!spriteSheet) {
        printf("No se pudo cargar la textura!\n");
        gfxExit();
        return 1;
    }

    /*Crear sprite*/
    C2D_Sprite sprite;
    C2D_SpriteFromSheet(&sprite, spriteSheet, 0);

    /*Escalar sprite para ocupar toda la pantalla superior (400 * 240)*/
    C2D_SpriteSetPos(&sprite, 0.0f, 0.0f);
    C2D_SpriteSetScale(&sprite, 1.0f, 1.0f);

    float mapX = 0.0f;  /*Posicion horizontal del mapa*/
    float mapY = 0.0f;  /*Posicion vertical del mapa*/

    float zoom = 1.0f; /*Nivel de zoom*/
    
    float mapWidth = 556.0f; /*Ancho de la imagen (mapa del metro)*/
    float mapHeight = 800.0f; /*Alto de la imagen (mapa del metro)*/

    /*Pantalla superior*/
    float screenWidth = 400.0f;
    float screenHeight = 240.0f;

    touchPosition touch;    /*Posicion actual del stylus*/
    touchPosition lastTouch;    /*Posicion anterior*/

    bool touching = false;  /*Saber si el stylus esta tocando la pantalla*/

    /*Texto de la pantalla inferior*/
    PrintConsole bottomScreen;
    consoleInit(GFX_BOTTOM, &bottomScreen);
    consoleSelect(&bottomScreen);
    printf("Mapa del metro de CDMX \n Desarrollado por Splinterino");
    /*Controles*/
    printf("\n\nPresiona Start para salir");
    printf("\nPresiona L para alejar el mapa");
    printf("\nPresiona R para acercar el mapa");
    printf("\n\nUtiliza el stylus para navegar!");

    /*Se ejecuta constantemente mientras la app esta activa*/
    /*Si necesita cerrarse devolvera false*/
    while (aptMainLoop())
    {
        /*Detecta que boton se presiona*/
        hidScanInput();

        hidTouchRead(&touch);
        u32 kHeld = hidKeysHeld();
        u32 kDown = hidKeysDown();
        u32 kUp = hidKeysUp();
        
        /*---Zoom con L y R---*/
        /*Acercamiento*/
        if (kDown & KEY_R) {
            zoom += 0.1f;
        }

        /*Alejamiento*/
        if (kDown & KEY_L) {
            zoom -= 0.1f;
        }

        /*---Limitamiento del zoom---*/
        /*Limitar zoom minimo*/
        if (zoom < 0.5f) {
            zoom = 0.5f;
        }

        /*Limitar zoom maximo*/
        if (zoom > 3.0f) {
            zoom = 3.0f;
        }

        /*Detecta cuando se usa el Stylus*/
        if (kDown & KEY_TOUCH) {
            lastTouch = touch;
            touching = true;
        }

        /*Instrucciones para la pantalla inferior cuando se detecte el Stylus*/
        if ((kHeld & KEY_TOUCH) && touching) {
            int dx = touch.px - lastTouch.px;
            int dy = touch.py - lastTouch.py;

            mapX += dx * 0.8f;
            mapY += dy * 0.8f;

            /*Limitar desplazamiento horizontal*/
            if (mapX > 0) {
                mapX = 0;
            }

            if (mapX < -(mapWidth * zoom - screenWidth)) {
                mapX = -(mapWidth * zoom - screenWidth);
            }

            /*Limitar desplazamiento vertical*/
            if (mapY > 0) {
                mapY = 0;
            }

            if (mapY < -(mapHeight * zoom - screenHeight)) {
                mapY = -(mapHeight * zoom - screenHeight);
            }

            lastTouch = touch;
        }

        if (kUp & KEY_TOUCH) {
            touching = false;
        }

        /*Si uno de los presionados es START entra al if*/
        if (kDown & KEY_START) {
            /*Salimos del while y se termina la aplicación*/
            break;
        }

        C2D_SpriteSetPos(&sprite, mapX, mapY);
        C2D_SpriteSetScale(&sprite, zoom, zoom);
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255)); /*Fondo negro*/
        C2D_SceneBegin(top);
        C2D_DrawSprite(&sprite);                          /*Dibujar mapa*/
        C3D_FrameEnd(0);
    }

    /*Liberamos recursos graficos y cerramos el sistema*/
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    
    return 0;
}