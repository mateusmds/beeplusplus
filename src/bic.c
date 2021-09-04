#define LSIZE (8 << 8) // max of bytes read per line

// error messages
imut char * REALERR = "an unexpected error occurred";
imut char * UNEXPCT = "unexpected symbol found here";
imut char * UNCLERR = "unclosed string found here";
imut char * CALLERR = "attempt to call an undefined function";
imut char * TOOFEWC = "too few arguments in the statement";
imut char * TOOMUCH = "too much arguments in the statement";
imut char * MULTIDF = "attempt to define multiple times the same namespace";
imut char * LITRIDX = "attempt to use a literal as namespace";
imut char * KWRDIDX = "attempt to use a keyword as namespace";

// array of source code lines
carr code;
// header-located string array
carr sarr;

void comp(FILE * fptr, char * outf, char * lddf, char * mthd){
    // compiling timer
    struct timespec crnt, oldt, diff;
    double dt;
    clock_gettime(CLOCK_REALTIME, &oldt);

    code.arr = malloc(sizeof(char *));
    puts("+loading code  ...");

    // get source
    u16 i = 0, csz = LSIZE;
    bool isinc = F;
    for(char c = 1; c = fgetc(fptr);){
        // skip non textual chars
        if(c < 9 and c >= 0) continue;

        // first char of line
        if(!i){
            // alloc line
            code.arr[code.len] = malloc(LSIZE);
            // resset length
            csz = LSIZE;
        }
        if(i < csz)
        if(c != '\n' and c != EOF)
            code.arr[code.len][i++] = c;
        else {
            code.arr[code.len][i] = '\0';
            i = 0;
            // remove newline
            code.arr[code.len] = strgsub(code.arr[code.len], "\n", "");

            if(!hassym(code.arr[code.len])){
                // default error arrow
                token arrw = {
                    .line  = code.len,
                    .coll  = strlen(code.arr[code.len]) - 1
                };
                cmperr("expected expression or terminator", &arrw, nil);
            }

            // check for both quote types
            for(u16 s = 0; s < 2; s++){
                u64 fidx = strfnd(code.arr[code.len], metachar[s].val);
                if(fidx != -1){
                    // the first up to the last quote
                    u64 lidx = strfndl(code.arr[code.len], metachar[s].val);

                    token arrow = {
                        .line = code.len,
                        .coll = strlen(code.arr[code.len]) - 1
                    };
                    if(lidx == -1) cmperr(UNCLERR, &arrow, nil);

                    // check count of quotes
                    u64 cnnt = 0;

                    // check if the quote count matches
                    for(u64 idx = fidx; idx <= lidx; idx++){
                        if(code.arr[code.len][idx] == metachar[s].val[0]
                        and code.arr[code.len][idx - 1] != '\\') cnnt++;
                    }

                    // missing quotes
                    if(cnnt % 2 != 0)
                    cmperr(UNCLERR, &arrow, nil);
                }
            }
            // remove comments
            i64 idx = strfnd(code.arr[code.len], SYMBOLS[SYM_CMM].e);

            if(idx > -1){
                char * sub = str_sub(code.arr[code.len], idx, -1);
                if(strcmp(sub, code.arr[code.len]))
                    code.arr[code.len] = strgsub(code.arr[code.len], sub, "");
                else {
                    free(code.arr[code.len]);
                    code.arr[code.len] = "";
                }
            }

            idx = strfnd(code.arr[code.len], SYMBOLS[SYM_MLC].s);
            if(idx > -1){
                if(!isinc){
                    char * sub = str_sub(code.arr[code.len], idx, -1);
                    if(strcmp(sub, code.arr[code.len]))
                        code.arr[code.len] = strgsub(code.arr[code.len], sub, "");
                    else {
                        free(code.arr[code.len]);
                        code.arr[code.len] = "";
                    }
                }
                isinc = T;
            }
            idx = strfnd(code.arr[code.len], SYMBOLS[SYM_MLC].e);
            if(idx > -1 and isinc){
                // string slice to be ignored
                char * sub = str_sub(
                    code.arr[code.len], 0,
                    idx + strlen(SYMBOLS[SYM_MLC].e) - 1
                );
                if(strcmp(sub, code.arr[code.len]))
                    code.arr[code.len] = strgsub(code.arr[code.len], sub, "");
                else {
                    free(code.arr[code.len]);
                    code.arr[code.len] = "";
                }
            }

            // next line
            code.arr = realloc(code.arr, (code.len + 2) * sizeof(char *));
            if(!code.arr) cmperr(REALERR, nil, nil);

            code.len++;

            // check it here to avoid
            // skipping the last char
            if(c == EOF) break;
        }
        // grow line
        else {
            csz += LSIZE / 2;
            code.arr[code.len] =
            realloc(code.arr[code.len], (csz) * sizeof(char *));

            if(!code.arr[code.len]) cmperr(REALERR, nil, nil);

            // first realloc
            if(csz == LSIZE * 1.5){
                token arrw = {
                    .line  = code.len,
                    .coll  = strlen(code.arr[code.len]) - 1
                };
                char * msg = malloc(80);
                sprintf(msg, "line too long! greater than %d bytes", LSIZE);
                wrning(msg, &arrw, nil);
                free(msg);
            }
        }
    }
    fclose(fptr);

    puts("+lexing code   ...");
    lexout tkns = lexit();

    for(u64 t = 0; t < tkns.tknc; t++){
        printf("[%s] %d %d %d\n",
        tkns.tkns[t].vall, tkns.tkns[t].line, tkns.tkns[t].coll, tkns.tkns[t].type);
        free(tkns.tkns[t].vall);
    }
    free(tkns.tkns);

    // compilation time
    clock_gettime(CLOCK_REALTIME, &crnt);
    diff.tv_sec  = crnt.tv_sec  - oldt.tv_sec;
    diff.tv_nsec = crnt.tv_nsec - oldt.tv_nsec;

    dt = (double)(diff.tv_nsec / 1000);
    dt /= 1000;
    dt += diff.tv_sec * 1000;

    printf("-compiled %s into %s in %.3fms\n", lddf, outf, dt);
}

bool isnumc(char chr){
    return isdigit(chr);
}

bool ishexc(char chr){
    return isxdigit(chr);
}

bool validn(char chr){
    return (('0' <= chr and chr <= '9') or
            ('A' <= chr and chr <= 'Z') or
            ('a' <= chr and chr <= 'z') or
    chr == '_');
}

bool matchs(char * str, bool(*func)(char)){
    bool valid = T;
    for(u64 c = 0; c < strlen(str); c++){
        if(valid) valid = func(str[c]);
        else break;
    }
    return valid;
}

bool hassym(char * str){
    u16 len = arrlen(SYMBOLS);
    for(u16 p = 0; p < len; p++){
        if(SYMBOLS[p].s != nil)
        if(endswith(str, SYMBOLS[p].s)) return T;

        if(endswith(str, SYMBOLS[p].e)) return T;
    }
    return F;
}

bool isscpd(char * str, u64 chr){
    if(chr < 0) return F;

    if(str[chr - 1] == '\\'
    and !isscpd(str, chr - 1)) return T;
    else return F;
}

i16 iskeyw(char * str){
    u16 len = arrlen(KEYWORDS);
    for(u16 kw = 0; kw < len; kw++){
        if(!strcmp(str, KEYWORDS[kw])) return kw;
    }
    return -1;
}

char * strtohex(char * data){
    // avoid too many calls
    u16 len = arrlen(metachar);
    for(u16 mc = 0; mc < len; mc++){
        data = strgsub(data, metachar[mc].key, metachar[mc].val);
    }

    // keep it in memory
    char temp[strlen(data) + 1];
    strcpy(temp, data);

    free(data);
    data = malloc(strlen(temp) * 2 + 3);
    sprintf(data, "0x");

    // convert string to hex code
    for(u64 c = 0; c < strlen(temp); c++){
        sprintf(data, "%s%.2x", data, temp[c]);
    }
    // keep length constant
    if(strlen(data) < 10) data = strpush(data, "00");

    return data;
}

char * strtoptr(char * str){
    if(!sarr.arr) sarr.arr = malloc(sizeof(char *));
    for(u64 s = 0; s < sarr.len; s++){
        if(!strcmp(str, sarr.arr[s])) return sarr.arr[s];
    }
    carr_push(sarr, str);
    return sarr.arr[sarr.len - 1];
}

lexout lexit(){
    // primitive ast output
    lexout out;
    out.tkns = malloc(sizeof(token));
    out.tknc = 0; // just to make sure

    // current line
    for(u64 l = 0; l < code.len; l++){

        u64 col = 0;               // last token's column
        u64 lvl = 0;               // scope level
        u64 lsz = strlen(code.arr[l]); // line length

        char * src = malloc(lsz + 1);
        strcpy(src, code.arr[l]);

        bool isstr = F; // is the current sentence a string?
        u64  ctp   = 0; // current token's pointer
        char ctkn[lsz];

        for(u64 c = 0; c < lsz; c++){
            // it's a token piece, just append
            if(validn(src[c])) ctkn[ctp++] = src[c];
            // do something now
            else {
                token this;

                this.type = UNKNOWN;
                this.coll = col;
                this.line = l;

                // ignore strings
                if(src[c] == metachar[0].val[0]
                or src[c] == metachar[1].val[0]){
                    bool issc = isscpd(src, c);
                    // in and out of string
                    if(!issc) isstr = !isstr;

                    if(!isstr){
                        char * dummy = malloc(strlen(ctkn) + 1);
                        strcpy(dummy, ctkn);

                        u64 len = arrlen(metachar);
                        for(u16 mc = 0; mc < len; mc++){
                            dummy = strgsub(dummy,
                            metachar[mc].key, metachar[mc].val);
                        }
                        // it's a doubleword char
                        if(src[c] == metachar[0].val[0]){
                            // slice it
                            if(strlen(dummy) > 4){
                                char * temp = dummy;
                                dummy = str_sub(dummy, 0, 4);
                                free(temp);
                            }
                            this.vall = strtohex(dummy);
                        // it's a string
                        } else {
                            // store the string literals at a
                            // table to be put in the head of
                            // the object file and in the tkn
                            // just store the pointer to it.
                            this.vall = strtoptr(dummy);
                        }

                        this.type = LITERAL;
                        ctp = 0;
                        goto finish;
                    }
                    // keep storing
                    else if(issc) ctkn[ctp++] = src[c];
                    continue;
                }
                // keep storing
                else if(isstr){
                    ctkn[ctp++] = src[c];
                    continue;
                }
                // keyword, index or literal
                else if(ctp > 0){
                    // just close the buffer
                    ctkn[ctp] = '\0';

                    i16 idx = iskeyw(ctkn);

                    // keyword
                    if(idx != -1)
                        this.type = KEYWORD;
                    else {
                        bool ish = F, isn = F, hashexdef;
                        u64 size = strlen(ctkn);

                        hashexdef = startswith(ctkn, "0x");

                        // validate the literals
                        if(hashexdef)
                        for(u16 chr = 2; chr < size; chr++){
                            ish = ishexc(ctkn[chr]);
                        }
                        else
                        for(u16 chr = 0; chr < size; chr++){
                            isn = isnumc(ctkn[chr]);
                        }

                        // hexadecimal literal
                        if(hashexdef){
                            if(ish)
                            this.type = LITERAL;
                            // invalid pattern
                            else
                            cmperr(UNEXPCT, &this, nil);
                        }
                        // decimal literal
                        else if(isn)
                            this.type = LITERAL;
                        // indexer
                        else this.type = INDEXER;
                    }
                    this.vall = malloc(strlen(ctkn) + 1);
                    strcpy(this.vall, ctkn);

                    // resset the current token
                    // pointer to the beginning
                    ctp = 0;

                    // decrement it if the
                    // current char may be 
                    // a symbol
                    if(src[c] != ' ') c--;
                }
                // symbol or operator
                else if(src[c] != ' '){
                    // just to make sure
                    this.vall = nil;

                    u16 alen = arrlen(SYMBOLS);
                    // compare with the known symbols
                    for(u16 s = 0; s < alen; s++){
                        // not all symbols are in pairs
                        if(SYMBOLS[s].s){
                            // set token to the same
                            // slice size of the sym
                            char t[alen + 1];
                            // avoid multiple calls
                            u16 len = strlen(SYMBOLS[s].s);

                            // append missing chars
                            for(u16 chr = c; chr < c + len; chr++){
                                t[chr - c] = src[chr];
                            }
                            t[len] = '\0';

                            if(!strcmp(SYMBOLS[s].s, t)){
                                this.vall = malloc(len + 1);
                                strcpy(this.vall, SYMBOLS[s].s);

                                this.type = LSYMBOL;
                                c += len - 1;
                                break;
                            }
                        }
                        // same of the last statement
                        u16 len = strlen(SYMBOLS[s].e);
                        char t[len + 1];

                        for(u16 chr = c; chr < c + len; chr++){
                            t[chr - c] = src[chr];
                        }
                        t[len] = '\0';

                        if(!strcmp(SYMBOLS[s].e, t)){
                            this.vall = malloc(len + 1);
                            strcpy(this.vall, SYMBOLS[s].e);

                            this.type = LSYMBOL;
                            c += len - 1;
                            break;
                        }
                    }
                    if(this.type != UNKNOWN) goto finish;

                    u16 len;
                    alen = arrlen(OPERATORS);
                    // compare with the known symbols
                    for(u16 o = 0; o < alen; o++){
                        // set token to the same
                        // slice size of the sym
                        char t[alen + 1];
                        // avoid multiple calls
                        len = strlen(OPERATORS[o]);

                        // append missing chars
                        for(u16 chr = c; chr < c + len; chr++){
                            t[chr - c] = src[chr];
                        }
                        t[len] = '\0';

                        // just check, but don't stop. it
                        // may occur some mismatches like
                        // =< being read as =, so keep it
                        // going until the end
                        if(!strcmp(OPERATORS[o], t)){
                            // avoid memory leaks
                            if(this.vall != nil) free(this.vall);

                            this.vall = malloc(len + 1);
                            strcpy(this.vall, OPERATORS[o]);

                            this.type = OPERATOR;
                        }
                    }
                    // move pointer after operator
                    c += len - 1;

                // just a space out of a string
                } else continue;

                finish:
                col = c + 1;
                tkn_push(out, this);
                if(!out.tkns) cmperr(REALERR, nil, nil);
            }
        }
        free(src);
    }
    return out;
}

bool vld_args(char * path){return T;}
bool vld_expr(char * path){return T;}
bool vld_scop(char * path){return T;}

void cmperr(imut char * err, token * arw, void * cmpl){
    // common prefix
    fprintf(stderr, "[ERROR] %s", err);
    if(arw)
    fprintf(stderr, " at %d:%d:\n", arw->line + 1, arw->coll + 1);
    else fprintf(stderr, "\n");

    if(arw){
        fprintf(stderr, "\n\t%d | %s\n\t", arw->line + 1, code.arr[arw->line]);
        fflush(stderr);

        for(u16 i = 0; i < arw->coll + 4; i++){fprintf(stderr, " ");}
        fflush(stderr);
        fprintf(stderr, "^\n");
    }
    fprintf(stderr, "press any key to exit...");
    scanf("nothing");
    exit(-1);
}

void wrning(imut char * wrn, token * arw, void * cmpl){
    printf("-warning: %d:%d at %s\n\t%s\n",
    arw->line, arw->coll, wrn);
}