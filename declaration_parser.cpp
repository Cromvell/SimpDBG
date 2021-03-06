
void init(Function_Declaration * decl) {
}

void deinit(Function_Declaration *decl) {
  decl->arguments.deinit();
}


Declaration_Token get_next_token(char *search_start) {
  switch (*search_start) {
  case '(':
    return (Declaration_Token){.kind=OPENING_PARENTHESE, .length=1};
  case ')':
    return (Declaration_Token){.kind=CLOSING_PARENTHESE, .length=1};
  case ',':
    return (Declaration_Token){.kind=COMMA_SEPARATOR, .length=1};
  case '*':
    return (Declaration_Token){.kind=STAR, .length=1};
  case '\0':
    return (Declaration_Token){.kind=EOS, .length=1};
  default: {
    char *pointer = search_start;

    if (!(isalpha(*pointer) || *pointer == '_'))  return (Declaration_Token){.kind=UNKNOWN_TOKEN, .length=-1};

    while (isalnum(*pointer) || *pointer == '_')  pointer++;

    if (strncmp(search_start, "const", 5) == 0) {
      return (Declaration_Token){.kind=CONST_SPECIFIER, .name=(char *)search_start, .length=static_cast <s32>(pointer - search_start)};
    } else {
      return (Declaration_Token){.kind=TYPE, .name=(char *)search_start, .length=static_cast <s32>(pointer - search_start)};
    }
  }
  }
}

Function_Declaration parse_function_declaration(char *string_start, u32 string_length) {
  Function_Declaration result;

  char *token_pointer = string_start;

  // @Note: Think about enforcing correctness of function identifier
  while ((token_pointer - string_start) < string_length && !isspace(*token_pointer) && *token_pointer != '(')  token_pointer++;

  result.function_name = string_start;
  result.name_length = token_pointer - string_start;

  while ((token_pointer - string_start) < string_length && *token_pointer != '(')  token_pointer++; // Skip to the opening parenthese

  if ((result.name_length + 1) >= string_length) {
    return result;
  } else {
    auto next_token = get_next_token(token_pointer);
    if (next_token.kind != Declaration_Token_Kind::OPENING_PARENTHESE)  return result;

    auto token = next_token;
    token_pointer = token_pointer + token.length;

    while (isspace(*token_pointer)) token_pointer++; // Skip spaces

    next_token = get_next_token(token_pointer);

    u8 stop_flags = (Declaration_Token_Kind::CLOSING_PARENTHESE | Declaration_Token_Kind::EOS | Declaration_Token_Kind::UNKNOWN_TOKEN);
    while (!(next_token.kind & stop_flags)) {
      switch (next_token.kind) {
      case Declaration_Token_Kind::TYPE:
        if (token.kind & (COMMA_SEPARATOR | OPENING_PARENTHESE | CONST_SPECIFIER | TYPE)) {
          if (token.kind == CONST_SPECIFIER) {
            result.arguments.add((Function_Argument){.type_name=next_token.name, .type_name_length=next_token.length, .is_const=true});
          } else if (token.kind == TYPE) {
            result.arguments.back().type_name_length = static_cast <s32>(next_token.name + next_token.length - result.arguments.back().type_name);
            result.arguments.back().is_compound_type = true;
          } else {
            result.arguments.add((Function_Argument){.type_name=next_token.name, .type_name_length=next_token.length});
          }
        } else {
          return (Function_Declaration){};
        }
        break;

      case Declaration_Token_Kind::STAR:
        if (result.arguments.count <= 0)  return (Function_Declaration){};

        if (token.kind == Declaration_Token_Kind::TYPE) {
          result.arguments.back().pointer_level = 1;
        } else if (token.kind == Declaration_Token_Kind::STAR && result.arguments.back().pointer_level > 0) {
          result.arguments.back().pointer_level++;
        } else {
          return (Function_Declaration){};
        }
        break;

      case Declaration_Token_Kind::COMMA_SEPARATOR:
        if (token.kind & (Declaration_Token_Kind::TYPE | Declaration_Token_Kind::STAR)) {
        } else {
          return (Function_Declaration){};
        }
        break;

      case Declaration_Token_Kind::OPENING_PARENTHESE:
        return (Function_Declaration){};

      case Declaration_Token_Kind::CONST_SPECIFIER:
        if (token.kind & (Declaration_Token_Kind::OPENING_PARENTHESE | Declaration_Token_Kind::COMMA_SEPARATOR)) {
        } else {
          return (Function_Declaration){};
        }
        break;
      }
      
      token = next_token;
      token_pointer = token_pointer + token.length;

      while (isspace(*token_pointer)) token_pointer++; // Skip spaces
      
      next_token = get_next_token(token_pointer);
    }

    if (next_token.kind == Declaration_Token_Kind::UNKNOWN_TOKEN)  return (Function_Declaration){};
    if (next_token.kind == Declaration_Token_Kind::CLOSING_PARENTHESE) {
      if (token.kind & (Declaration_Token_Kind::TYPE | Declaration_Token_Kind::STAR | Declaration_Token_Kind::OPENING_PARENTHESE)) {
      } else {
        return (Function_Declaration){};
      }
    }
    if (next_token.kind == Declaration_Token_Kind::EOS) {
      if (token.kind == Declaration_Token_Kind::CLOSING_PARENTHESE) {
      } else {
        return (Function_Declaration){};
      }
    }

  }

  return result;
}
