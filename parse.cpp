//---------------------------------------------------------------------------


#pragma hdrstop

#include "parse.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

parser::parser() //�����������
{
  int i;

  exp_ptr = NULL; //exp_ptr=0

  for ( i = 0; i < NUMVARS; i++ ) vars[i] = 0.0; //��������� ������� vars[ ]
}

double parser::process_stage_main( char * exp ) //������� ������ ����������
{
  double result; //���������� � ������� ����� ������� ���������
  exp_ptr = exp;

  get_element(); //������� ��������� ��������
  if ( ! * element )
  {
    print_error( 2 );
    return 0.0;
  }
  process_stage_1( result );
  if ( * element )
  {
    print_error( 0 );
  }
  return result;
}

void parser::process_stage_1( double & result )
{
  int slot;
  char ttok_type;
  char temp_element[80];

  if ( tok_type == VARIABLE )
  {
    strcpy( temp_element, element );
    ttok_type = tok_type;
    slot = toupper( * element ) - 'A';
    get_element(); //�������� �������
    if ( * element != '=' )
    {
      putback();
      strcpy( element, temp_element );
      tok_type = ttok_type;
    }
    else
    {
      get_element(); //�������� �������
      process_stage_2( result );
      vars[slot] = result;
      return;
    }
  }
  process_stage_2( result );
}

void parser::process_stage_2( double & result )
{
  register char op; //���������� � ��������� �������� �����
  double temp;

  process_stage_3( result );
  while ( ( op = * element ) == '+' || op == '-' )
  {
    get_element(); //�������� �������
    process_stage_3( temp );
    switch ( op ) //switch - �������� ������:
    {
      case '-': //���� op == '-'
        result = result - temp;
      break;
      case '+': //���� op == '+'
        result = result + temp;
      break;
    }
  }
}

void parser::process_stage_3( double & result )
{
  register char op; //���������� � ��������� �������� �����
  double temp;
  process_stage_4( result );
  while ( ( op = * element ) == '*' || op == '/' || op == '%' )
  {
    get_element(); //�������� �������
    process_stage_4( temp );
    switch ( op ) //switch - �������� ������:
    {
      case '*': //���� op == '*'
        result = result * temp;
      break;
      case '/': //���� op == '/'
        result = result / temp;
      break;
      case '%': //���� op == '%'
        result = ( int )result % ( int )temp;
      break;
    }
  }
}

void parser::process_stage_4( double & result )
{
  double temp, ex;

  process_stage_5( result );
  if ( * element == '^' )
  {
    get_element(); //�������� �������
    process_stage_4( temp );
    ex = result;
    result = pow( ex, temp );
  }
}

void parser::process_stage_5( double & result )
{
  register char op;

  op = 0;
  if ( ( tok_type == DELIMITER ) && ( * element == '+' || * element == '-' ) )
  {
    op = * element;
    get_element(); //�������� �������
  }
  process_stage_6( result );
  if ( op == '-' ) result = -result;

}

void parser::process_stage_6( double & result )
{
  if ( ( * element == '(' ) )
  {
    get_element();
    process_stage_2( result );
    if ( * element != ')' )
      print_error( 1 );
    get_element(); //�������� �������
  }
  else if ( ( * element == '[' ) )
  {
    get_element();
    process_stage_2( result );
    result = fabs( result );
    if ( * element != ']' )
      print_error( 1 );
    get_element(); //�������� �������
  }
  else
    get_value( result );
}

void parser::get_value( double & result )
{
  switch ( tok_type ) //switch - �������� ������
  {
    case VARIABLE: //���� tok_type == VARIABLE
      result = search_vars( element );
      get_element();
      return;
    case NUMBER: //���� tok_type == NUMBER
      result = atof( element ); //atof - ����������� ������ � ����� � ��������� �������
      get_element(); //�������� �������
      return;
    default: //�� ���������
      print_error( 0 );
  }
}

void parser::putback()
{
  char * t;
  t = element;
  for ( ; * t; t++ ) //����, ����������� ���� ���������� ��������� * t
    exp_ptr--;
}

void parser::print_error( int error ) //������� ������ ������
{
  static char * e[] =
  {
    "Sintax Error", "UnClosed Skobka""No Expression to compile"
  };
  cout << e[error] << endl;
}

void parser::get_element() //������� ��������� ��������
{
  register char * temp; //���������� � ��������� �������� �����
  tok_type = 0;
  temp = element;
  * temp = '\0';

  if ( ! * exp_ptr ) return;

  while ( isspace( * exp_ptr ) ) //������ �������� �� ���������� ������ (������, ���������, ����� ������)
    ++exp_ptr;

  if ( strchr( "+-*/%^=()[]", * exp_ptr ) ) //��������� ( char *strchr(const char * s, int c) ) - ���������� ��������� �� ������ ��������� c � s, ��� NULL
  {
    tok_type = DELIMITER;
    * temp++ = * exp_ptr++;
  }
  else if ( isalpha( * exp_ptr ) ) //��������� ( int isalpha(int c) ) - ������ �������� �� ��������� �����
  {
    while ( !isdelim( * exp_ptr ) ) * temp++ = * exp_ptr++;
    if ( * element != 'pi' )
    {
      tok_type = VARIABLE;
    }
    else
    {
      tok_type = TRIG;
    }
  }
  else if ( isdigit( * exp_ptr ) )
  {
    while ( !isdelim( * exp_ptr ) ) * temp++ = * exp_ptr++;
    tok_type = NUMBER;
  }
  * temp = '\0';
}

int parser::isdelim( char c )
{
  if ( strchr( " +-/*%^=()[]", c ) || c == 9 || c == '\r' || c == 0 ) //��������� ( char *strchr(const char * s, int c) ) -
  // ���������� ��������� �� ������ ��������� c � s, ��� NULL
    return 1; //���� ��, �� ���������� 1
  return 0; //����� 0
}

double parser::search_vars( char * s )
{
  if ( !isalpha( * s ) ) //��������� ( int isalpha(int c) ) - ������ �������� �� ��������� �����
  {
    print_error( 1 );
    return 0.0;
  }
  return vars[toupper( * element ) - 'A']; //toupper - ������ ���������� ��������� ����� � �������� ��������, ���� ��� � ������ ��������

}
//------------------------------------------------------------------------------
