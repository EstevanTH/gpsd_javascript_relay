/** Class ValidatorJavascriptFunction
This class validates the name of a JavaScript function.
**/

#ifndef VALIDATORJAVASCRIPTFUNCTION_H
#define VALIDATORJAVASCRIPTFUNCTION_H

#include <QValidator>

class ValidatorJavascriptFunction: public QValidator{
	public:
		void fixup(QString &input) const;
		QValidator::State validate(QString &input, int &pos) const;
};

#endif // VALIDATORJAVASCRIPTFUNCTION_H
