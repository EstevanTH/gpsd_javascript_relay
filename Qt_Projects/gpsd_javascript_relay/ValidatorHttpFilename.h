/** Class ValidatorHttpFilename
This class validates the name of an HTTP filename.
**/

#ifndef VALIDATORHTTPFILENAME_H
#define VALIDATORHTTPFILENAME_H

#include <QValidator>

class ValidatorHttpFilename: public QValidator{
	public:
		void fixup(QString &input) const;
		QValidator::State validate(QString &input, int &pos) const;
};

#endif // VALIDATORHTTPFILENAME_H
