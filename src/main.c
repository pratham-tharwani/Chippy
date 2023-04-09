#include <stdlib.h>
#include <stdio.h>

unsigned short opcode;
unsigned char memory[4096];
unsigned char V[16];
unsigned short pc;
unsigned short I;
unsigned char gfx[64*32];
unsigned char sound_timer;
unsigned char delay_timer;
unsigned short stack[16];
unsigned short sp;
unsigned char key[16];
unsigned int drawflag = 0;

unsigned char fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize()
{
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for(int i = 0; i < 64*32; i++)
    {
        gfx[i] = 0;
    }

    for(int i = 0; i < 16; i++)
    {
        V[i] = 0;
        stack[i] = 0;
        key[i] = 0;
    }

    for(int i = 0; i < 4096; i++)
    {
        memory[i] = 0;
    }

    for(int i = 0; i < 80; i++)
    {
        memory[i] = fontset[i];
    }

    sound_timer = 0;
    delay_timer = 0;
    drawflag = 1;
}

void load_game(char* filename)
{
    FILE* fptr = fopen(filename, "rb");

    if(fptr == NULL)
    {
        printf("Could not open file.\n");
        return;
    }

    fseek(fptr, 0L, SEEK_END);
    int sz = ftell(fptr);
    rewind(fptr);

    char buf[sz];

    fread(buf, sizeof(char), sz, fptr);

    for(int i = 0; i < sz; i++)
    {
        memory[i + 0x200] = buf[i];
    }

    fclose(fptr);
}

void emulate_cycle()
{
    opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;

    unsigned char X = (opcode & 0x0F00) >> 8;
    unsigned char Y = (opcode & 0x00F0) >> 4;
    unsigned char N = opcode & 0x000F;
    unsigned char NN = opcode & 0x00FF;
    unsigned short NNN = opcode & 0x0FFF;

    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (N)
        {
            case 0x0000:
                for(int i = 0; i < 64*32; i++)
                {
                gfx[i] = 0;
                }
                drawflag = 1;
                break;
            
            case 0x000E:
                pc = stack[sp];
                sp--;
                break;
        }
        break;
    
    case 0x1000:
        pc = NNN;
        break;
    
    case 0x2000:
        sp++;
        stack[sp] = pc;
        pc = NNN;
        break;
    
    case 0x3000:
        if(V[X] == NN)
        {
            pc += 2;
        }
        break;
    
    case 0x4000:
        if(V[X] != NN)
        {
            pc += 2;
        }
        break;
    
    case 0x5000:
        if(V[X] == V[Y])
        {
            pc += 2;
        }
        break;
    
    case 0x6000:
        V[X] = NN;
        break;
    
    case 0x7000:
        V[X] += NN;
        break;
    
    case 0x8000:
        int flag;
        switch (N)
        {
            case 0x0000:
                V[X] = V[Y];
                break;
            
            case 0x0001:
                V[X] = V[X] | V[Y];
                V[0xF] = 0;
                break;
            
            case 0x0002:
                V[X] = V[X] & V[Y];
                V[0xF] = 0;
                break;
            
            case 0x0003:
                V[X] = V[X] ^ V[Y];
                V[0xF] = 0;
                break;
            
            case 0x0004:
                if(V[Y] > 0xFF - V[X])
                {
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }

                V[X] = V[X] + V[Y];
                V[0xF] = flag;
                break;
            
            case 0x0005:
                if(V[X] > V[Y])
                {
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }

                V[X] = V[X] - V[Y];
                V[0xF] = flag;
                break;
            
            case 0x0006:
                V[X] = V[Y];
                flag = V[X] & 0x1;
                V[X] >>= 1;
                V[0xF] = flag;
                break;
            
            case 0x0007:
                if(V[Y] > V[X])
                {
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }

                V[X] = V[Y] - V[X];
                V[0xF] = flag;
                break;
            
            case 0x000E:
                V[X] = V[Y];
                flag = V[X] >> 7;
                V[X] <<= 1;
                V[0xF] = flag;
                break;
        }
        break;
    
    case 0x9000:
        if(V[X] != V[Y])
        {
            pc += 2;
        }
        break;
    
    case 0xA000:
        I = NNN;
        break;
    
    case 0xB000:
        pc = V[0] + NNN;
        break;
    
    case 0xC000:
        V[X] = (rand() % 256) & NN;
        break;
    
    case 0xD000:
        unsigned short xcoord = V[X] % 64;
        unsigned short ycoord = V[Y] % 32;
        V[0xF] = 0;
        unsigned short pixel;

        for(int yline = 0; yline < N; yline++)
        {
            pixel = memory[I + yline];

            for(int xline = 0; xline < 8 ; xline++)
            {
                int xpos = xcoord + xline;
                int ypos = (ycoord + yline);

                if((xpos > 63) || (ypos > 31))
                {
                    break;
                }

                if((pixel & (0x80 >> xline)) != 0)
                {
                    if(gfx[xpos + ypos*64] == 1)
                    {
                        V[0xF] = 1;
                    }
                    gfx[xpos + ypos*64] ^= 1;
                }
            }
        }

        drawflag = 1;
        break;
    
    case 0xE000:
        switch (opcode & 0x00F0)
        {
        case 0x0090:
            if(key[V[X]] != 0)
            {
                pc += 2;
            }
            break;
        

        case 0x00A0:
            if(key[V[X]] == 0)
            {
                pc += 2;
            }
            break;
        }
    
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            V[X] = delay_timer;
            break;
        
        case 0x000A:
            unsigned int keypressed = 0;

            for(int i = 0; i < 16; i++)
            {
                if(key[i] != 0)
                {
                    V[X] = i;
                    keypressed = 1;
                }
            }

            if(keypressed == 0)
            {
                pc -= 2;
            }
            break;
        
        case 0x0015:
            delay_timer = V[X];
            break;
        
        case 0x0018:
            sound_timer = V[X];
            break;
        
        case 0x001E:
            I = I + V[X];
            break;
        
        case 0x0029:
            I = V[X] * 5;
            break;
        
        case 0x0033:
            memory[I] = V[X] / 100;
            memory[I+1] = (V[X]/10) % 10;
            memory[I+2] = V[X] % 10;
            break;
        
        case 0x0055:
            for(int i = 0; i <= X; i++)
            {
                memory[I++] = V[i];
            }
            break;
        
        case 0x0065:
            for(int i = 0; i <= X; i++)
            {
                V[i] = memory[I++];
            }
            break;
        }
        break;

    default:
        printf("Unknown opcode: %x\n", opcode);
        break;
    }

    if(delay_timer > 0)
    {
        delay_timer--;
    }

    if(sound_timer > 0)
    {
        printf("BEEP!\n");
        sound_timer--;
    }
}

int main(int argc, char** argv)
{
    initialize();
    memory[0x1FF] = 1;
    load_game(argv[1]);

    while(1)
    {
        emulate_cycle();

        if(drawflag)
        {
            for(int i = 0; i < 32; i++)
            {   
                for(int j = 0; j < 64; j++)
                {
                    printf("%s", gfx[i*64 + j] ? "\u2588\u2588" : "  ");
                }
                printf("\n");
            }
            drawflag = 0;            
        }
    }
}