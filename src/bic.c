#define LSIZE (64 << 1) // max of bytes read per line

// error messages
imut char * REALERR = "an unexpected error occurred";
imut char * UNEXPCT = "unexpected symbol found here";
imut char * EXPCTDS = "expected expression or terminator";
imut char * UNCLSTR = "unclosed string found here";
imut char * UNCCMMT = "unclosed multiline comment found here";
imut char * UNCBRCK = "unclosed bracket found here";
imut char * UNCPARN = "unclosed parentheses found here";
imut char * EXPCTDP = "expected a parentheses at the end of this expression";
imut char * PRVONHR = "previously opened here";
imut char * ALONEXP = "expression out of context found here";
imut char * CALLERR = "attempt to call an undefined function";
imut char * TOOFEWC = "too few arguments in the statement";
imut char * TOOMUCH = "too much arguments in the statement";
imut char * MULTIDF = "attempt to define multiple times the same namespace";
imut char * LITRIDX = "attempt to use a literal as namespace";
imut char * KWRDIDX = "attempt to use a keyword as namespace";
imut char * KWRDVAL = "attempt to use a keyword as value";
imut char * EXPCTEX = "expected expression at this point";
imut char * DEFWOEQ = "you cannot assign a variable with shorthand operators while defining it";
imut char * NOTASGN = "this is not a valid assignment operator";
imut char * MSMATCH = "mismatch of number of assignators and assignateds in expression";
imut char * NOTERMN = "no terminator at the end of the line";

// string array of each line of code
stra code;

cout * comp(FILE * fptr, char * lddf, char * mode){
    // output
    cout * out = malloc(sizeof(out));

    char * dummy = malloc(strlen(lddf) + 1);
    strcpy(dummy, lddf);
    out->outn  = strgsub(dummy, ".bi", "");

    // compiling timer
    clock_t crnt, oldt = clock();
    f32 dt;

    COL(GRN);
    printf("+loading code  ...");
    COL(DEF);

    // load code
    code = load(fptr);

    crnt = clock();
    dt = ((double)(crnt - oldt)) / ((double)CLOCKS_PER_SEC);
    
    COL(GRN);
    printf(" done %sin %s%.5fs\n", DEF, BLU, dt);
    COL(DEF);

    COL(GRN);
    printf("+lexing code   ...");
    COL(DEF);

    tkn * tkns = lexit();

    crnt = clock();
    dt = ((double)(crnt - oldt)) / ((double)CLOCKS_PER_SEC);

    COL(GRN);
    printf(" done %sin %s%.5fs\n", DEF, BLU, dt);
    COL(DEF);

    COL(GRN);
    printf("+parsing code  ...");
    COL(DEF);

    cmod cmd;
    if(!strcmp(mode, "check")) cmd = CHECK;
    else if (!strcmp(mode, "debug")) cmd = DEBUG;
    else cmd = BUILD;

    // basic ast
    node * bast = parse(tkns, cmd);
    if(cmd == CHECK) goto skip_cmp;

    crnt = clock();
    dt = ((double)(crnt - oldt)) / ((double)CLOCKS_PER_SEC);

    COL(GRN);
    printf(" done %sin %s%.5fs\n", DEF, BLU, dt);
    COL(DEF);

    // TODO: final ast gen
    if(cmd == DEBUG) goto skip_opt;
    // TODO: optimizations
    skip_opt:
    // TODO: NASM gen

    skip_cmp:

    node *temp, *this = bast->stt;
    while(T){
        temp = this;
        this = temp->next;

        if(temp == bast->end){
            free_node(temp);
            break;    
        } else free_node(temp);
    }
    free(bast);

    tkn * tok, * old = nil;
    i64   cnt = EOTT->apdx;
    // free tokens
    for(tok = tkns; cnt > 0; tok = tok->next, cnt--){
        // only free string values
        if(tok->type == INDEXER)
            free(tok->vall.str);

        // only free char literals
        else if(tok->type == LITERAL and tok->apdx == FREEABLE)
            free(tok->vall.str);

        if(old) free(old);
        old = tok;
    }
    free(old);
    free_str();

    // compilation time
    crnt = clock();
    dt = ((double)(crnt - oldt)) / ((double)CLOCKS_PER_SEC);

    printf("-compiled %s into %s in ", lddf, out->outn);
    COL(GRN);
    printf("%.5fs\n", dt);
    COL(DEF);

    return out;
}

void free_node(node * n){
    cnst u16 cnnt = 0;
    char * pout;
    if(!n->is_parent) free(n);
    else {
        cnnt++;
        node * tmp, *ths = n->stt;
        assert(ths != nil, nodet_to_str(n));
        while(T){
            tmp = ths;
            ths = tmp->next;
            
            pout = nodet_to_str(tmp);
            printf("%s\n", pout); // This segfaults due to invalid pointer
            free(pout);

            if(tmp == n->end){
                free_node(tmp);
                puts("===");
                break;
            } else free_node(tmp);
        }
        free(n);
    }
}

void cmperr(imut char * err, tkn * arw, tkn * cmpl){
    // flush compilation messages
    printf("\n\n");

    // common prefix
    fprintf(stderr, RED);
    fprintf(stderr, "[ERROR]");
    fprintf(stderr, DEF);

    fprintf(stderr, " %s", err);
    // print arrow
    if(arw){
        fprintf(stderr, " at %ld:%ld:\n", arw->line + 1, arw->coln + 1);
        fprintf(stderr, BLU);

        char stt[code.lgrst];
        sprintf(stt, "\n\t%ld | ", arw->line + 1);

        fprintf(stderr, "%s%s\n\t", stt, code.arr[arw->line]);
        fprintf(stderr, DEF);
        fflush(stderr);

        fprintf(stderr, RED);
        for(u16 i = 0; i < arw->coln + strlen(stt) - 2; i++){
            if(i > 3) fprintf(stderr, "~");
            else fprintf(stderr, " ");
        }
        fflush(stderr);

        fprintf(stderr, "^\n");
        fprintf(stderr, DEF);

    // just end it
    } else fprintf(stderr, "\n");

    if(cmpl != nil){
        fprintf(stderr, "%s", cmpl->vall.str);

        fprintf(stderr, " at %ld:%ld:\n",
        cmpl->line + 1, cmpl->coln + 1);

        char stt[code.lgrst];
        sprintf(stt, "\n\t%ld | ", cmpl->line + 1);

        fprintf(stderr, BLU);
        fprintf(stderr, "%s%s\n\t", stt, code.arr[cmpl->line]);
        fprintf(stderr, DEF);
        fflush(stderr);

        fprintf(stderr, RED);
        for(u16 i = 0; i < cmpl->coln + 4; i++){
            if(i > 3) fprintf(stderr, "~");
            else fprintf(stderr, " ");
        }
        fflush(stderr);

        fprintf(stderr, "^\n");
        fprintf(stderr, DEF);
    }
    fprintf(stderr, "press any key to exit...");
    scanf("nothing");
    exit(-1);
}

void wrning(imut char * wrn, tkn * arw, tkn * cmpl){
    COL(YEL);
    printf("[WARNING]");
    COL(DEF);

    printf(" at %ld:%ld\n\t%s\n",
    arw->line, arw->coln, wrn);
}