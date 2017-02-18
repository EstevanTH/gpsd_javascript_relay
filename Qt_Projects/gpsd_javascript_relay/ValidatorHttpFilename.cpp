#include "ValidatorHttpFilename.h"

#define IS_VALID_CHAR_HTTP( chr ) \
	(  ( ( chr )>='0' && ( chr )<='9' ) \
	|| ( ( chr )>='A' && ( chr )<='Z' ) \
	|| ( ( chr )>='a' && ( chr )<='z' ) \
	|| ( ( chr )=='_' ) \
	|| ( ( chr )=='-' ) \
	|| ( ( chr )=='.' ) \
	|| ( ( chr )=='/' ) )

void ValidatorHttpFilename::fixup(QString &input) const{
	QString const& source = input;
	QString fixedInput;
	bool fixOccurred = false;
	for( QString::const_iterator chr2=source.begin(); chr2!=source.end(); ++chr2 ){
		char chr = ( *chr2 ).toLatin1();
		if( IS_VALID_CHAR_HTTP( chr ) ){
			fixedInput += *chr2;
		}
		else{
			fixOccurred = true;
		}
	}
	if( fixOccurred ){
		input = fixedInput;
	}
}

QValidator::State ValidatorHttpFilename::validate(QString &input, int &pos) const{
	QString const& source = input;
	for( QString::const_iterator chr2=source.begin(); chr2!=source.end(); ++chr2 ){
		char chr = ( *chr2 ).toLatin1();
		if( !IS_VALID_CHAR_HTTP( chr ) ){
			return QValidator::Invalid;
		}
	}
	if( input.isEmpty() ){
		return QValidator::Intermediate;
	}
	return QValidator::Acceptable;
	( void )pos; // unused
}
