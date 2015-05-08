#ifndef CALLBACK_H
#define CALLBACK_H

//http://www.codeguru.com/cpp/cpp/cpp_mfc/callbacks/article.php/c4129/C-Callback-Demo.htm

class cCallback
{
    public:
		virtual ~cCallback() {};
        virtual bool Execute() const =0;
};


template <class cInstance>
class TCallback : public cCallback
{
    public:
        TCallback()    // constructor
        {
        	cInst = NULL;
            pFunction = 0;
        }

        virtual ~TCallback()
        {
        	cInst = NULL;
        	pFunction = 0;
        }

        typedef bool (cInstance::*tFunction)();

        virtual bool Execute() const
        {
            if (pFunction) {
            	return (cInst->*pFunction)();
            }
            else {
            	return false;
            }
        }

        void SetCallback (cInstance  *cInstancePointer,
                          tFunction   pFunctionPointer)
        {
            cInst     = cInstancePointer;
            pFunction = pFunctionPointer;
        }

    private:
        cInstance  *cInst;
        tFunction  pFunction;
};

#endif //CALLBACK_H
