#include "ValidatorJavascriptFunction.h"

#define IS_VALID_CHAR_JS_FUNCTION( chr, pos ) \
	(  ( ( chr )>='0' && ( chr )<='9' && ( pos )!=0 ) \
	|| ( ( chr )>='A' && ( chr )<='Z' ) \
	|| ( ( chr )>='a' && ( chr )<='z' ) \
	|| ( ( chr )=='_' ) )

void ValidatorJavascriptFunction::fixup(QString &input) const{
	QString const& source = input;
	QString fixedInput;
	bool fixOccurred = false;
	int cursorFixed = 0;
	for( QString::const_iterator chr2=source.begin(); chr2!=source.end(); ++chr2 ){
		char chr = ( *chr2 ).toLatin1();
		if( IS_VALID_CHAR_JS_FUNCTION( chr, cursorFixed ) ){
			fixedInput += *chr2;
			++cursorFixed; // forbid every heading digit
		}
		else{
			fixOccurred = true;
		}
	}
	if( fixOccurred ){
		input = fixedInput;
	}
}

QValidator::State ValidatorJavascriptFunction::validate(QString &input, int &pos) const{
	QString const& source = input;
	int cursor = 0;
	for( QString::const_iterator chr2=source.begin(); chr2!=source.end(); ++chr2 ){
		char chr = ( *chr2 ).toLatin1();
		if( !IS_VALID_CHAR_JS_FUNCTION( chr, cursor ) ){
			return QValidator::Invalid;
		}
		++cursor;
	}
	return QValidator::Acceptable;
	( void )pos; // unused
}
