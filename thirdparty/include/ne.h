/*
 *   ICT Lexical Analysis Platform
 *
 *       Liu Bingyang
 *       Liu Qian
 *       Zhong Yanqin
 *       Wu Dayong
 *
 *   File Creator: Liu Bingyang
 *   Create Time: 2013-05-09
 *
 */


#ifndef ICT_LA_PLATFORM_CORE_NE
#define ICT_LA_PLATFORM_CORE_NE

#include <list>
#include <string.h>

#ifdef WIN32
    #ifdef ICTLAP_CORE_NE_EXPORT
        #define ICTLAP_CORE_NE_DLL _declspec (dllexport)
    #else
        #define ICTLAP_CORE_NE_DLL _declspec (dllimport)
    #endif
#else
    #define ICTLAP_CORE_NE_DLL
#endif

namespace ICTLAP {
    namespace CORE {

        /*Named Entity Recognition*/
        namespace NE {

            /*人名， 机构名， 地名， 国家名， 股票名， 时间*/
            enum NEtype { peo, org, loc, locC, stock, neTime};

            // containMatrix[i][j] means type i can contain type j
            static bool containMatrix[6][6] = {
                0, 0, 0, 0, 0, 0,
                1, 0, 1, 1, 1, 0,
                1, 0, 0, 1, 0, 0,
                0, 0, 0, 0, 0, 0,
                0, 0, 1, 1, 0, 0,
                0, 0, 0, 0, 0, 0
            };

            /*
            annoteNode表示一个返回的命名实体
            --offset: 在原文中的偏移量
            --len:    命名实体长度(utf-8 字节数，例如：“中国a”，长度为3+3+1=7)
            --tp:     命名实体类型
            */
            class annoteNode {
            public:
                long    offset;
                long    len;
                NEtype  tp;
                bool    isDisabled;
                annoteNode() {
                    offset = len = 0;
                    isDisabled = 0;
                }
                bool operator < (const annoteNode &t) const {
                    if(this->offset < t.offset) return true;
                    if(this->offset > t.offset) return false;

                    if(this->len < t.len) return true;
                    if(this->len > t.len) return false;

                    return this->tp < t.tp;
                }
                bool operator == (const annoteNode &t) const {
                    return ( this->offset  == t.offset &&
                        this->len == t.len &&
                        this->tp == t.tp);
                }
            };

            // 命名实体结果结构体
            class NEresult {
                char* _origText;
                char* _toStr;
                char* _error;

              public:
                /*文本中所有的命名实体*/
                std::list<annoteNode> labels;

                NEresult();
                NEresult(const char*);
                ~NEresult();
                void uniqueSort();
                void reverseSort();
                void innerPrint();

                /*
                toString()
                使用在原文中插入标签的方式返回原文，例如：
                “美国总统奥巴马” 返回 <lc>美国</lc>总统<n>奥巴马</n>
                */

                const char* toString(){
                    return _toStr;
                }
                const char* getOrigText() {
                    return _origText;
                }
                const char* errorInfo() {
                    return _error;
                }
                void setErrorInfo(const char* ei) {
                    if(_error != NULL) delete[] _error;
                    _error = NULL;
                    _error = new char[strlen(ei) + 16];
                    if(_error!=NULL) {
                        _error[0] = 0;
                        strcpy(_error, ei);
                    }
                }
                void conflictResolve();
            };


            typedef void* NEapi;
            typedef NEresult* NEres;

            /*!
             *  \fn ICTLAP_CORE_NE_DLL NEapi Init(const char* confFileName)
             *  \brief 命名实体模块全局初始化
             *  \param [in] confFileName 配置文件全路径
             *  \return 成功返回非空指针，否则返回NULL
             *  \author liubingyang
             */
            ICTLAP_CORE_NE_DLL NEapi Init(const char* confFileName);

            /*!
             *  \fn ICTLAP_CORE_NE_DLL NEapi Init2(const char* confFileName, const char* logFile)
             *  \brief 命名实体模块全局初始化
             *  \param [in] confFileName 配置文件全路径
             *  \param [in] logFileName 覆盖配置文件中的log路径
             *  \return 成功返回非空指针，否则返回NULL
             *  \author liubingyang
             */
            ICTLAP_CORE_NE_DLL NEapi Init2(const char* confFileName, const char* logFileName);

            /*!
             *  \fn ICTLAP_CORE_NE_DLL NEapi Init3(const char* confFileName, const char* logFile, int port)
             *  \brief 命名实体模块全局初始化
             *  \param [in] confFileName 配置文件全路径
             *  \param [in] logFileName 覆盖配置文件中的log路径
             *  \param [in] port 传入端口号
             *  \return 成功返回非空指针，否则返回NULL
             *  \author liubingyang
             */
            ICTLAP_CORE_NE_DLL NEapi Init3(const char* confFileName, const char* logFileName, int port);

            /*!
             *  \fn ICTLAP_CORE_NE_DLL void getSharedConf(NEapi h, const char* confKey, char* confValue)
             *  \brief 在共享core_ne的配置文件的情况下，取得core_ne的配置文件项
             *  \param [in] h Init初始化返回的指针
             *  \param [in] confKey 配置项名称
             *  \param [out] confValue 取回配置项值，需要自行分配空间
             *  \return void
             *  \author liubingyang
             */
            ICTLAP_CORE_NE_DLL void getSharedConf(NEapi h, const char* confKey, char* confValue);

            /*!
             * \fn ICTLAP_CORE_NE_DLL NEres Parse(NEapi h, const char* text, long len)
             * \brief 计算命名实体结果主函数
             * \param [in] h Init初始化返回的指针
             * \param [in] text 需要处理的文本
             * \param [in] len 文本的长度
             * \return 命名实体结果结构体指针
             * \author liubingyang
             */
            ICTLAP_CORE_NE_DLL NEres Parse(NEapi h, const char* text, long len);

            /*!
             * \fn ICTLAP_CORE_NE_DLL void  ReleaseRes(NEapi h, NEres)
             * \brief 释放命名实体结果空间
             * \param [in] h Init初始化返回的指针
             * \param [in] NEres Parse返回的结果指针
             * \return 无
             * \author liubingyang
             */
            ICTLAP_CORE_NE_DLL void  ReleaseRes(NEapi h, NEres);

            /*!
             * \fn ICTLAP_CORE_NE_DLL void  Exit(NEapi h)
             * \brief 命名实体模块全局退出
             * \param [in] h Init初始化返回的指针
             * \return 无
             * \author liubingyang
             */
            ICTLAP_CORE_NE_DLL void  Exit(NEapi h);

            /*!
             * \fn ICTLAP_CORE_NE_DLL void Version()
             * \brief 打印版本信息
             * \return 无
             * \author liubingyang
             */
            ICTLAP_CORE_NE_DLL void  Version();

        }
    }
}


/*********************************************************************************/
extern "C" {
    // this part is for cross language lib reference
    // use extern C can generate funtions with static names
    // Another reason is extern "C" is not compatitable with namespaces
    // only basic types of variables are used here

    /*
     * 此部分是为了跨语言的调用和链接
     * 使用 extern C 是为了在链接表中输出固定不变的名字
     * 此外 exterc "C" 和 namespace 不能兼容，所以本部分的函数的
     * 功能与上面的函数完全相同但是命名更长防止冲突
     */
    typedef void* CORE_NE_API;

    ICTLAP_CORE_NE_DLL CORE_NE_API CORE_NE_Init(const char* confFileName);
    ICTLAP_CORE_NE_DLL CORE_NE_API CORE_NE_Init2(const char* confFileName, const char* logFileName);
    ICTLAP_CORE_NE_DLL CORE_NE_API CORE_NE_Init3(const char* confFileName, const char* logFileName, int port);
    ICTLAP_CORE_NE_DLL char* CORE_NE_Parse(CORE_NE_API h, const char* text, long len);
    ICTLAP_CORE_NE_DLL void CORE_NE_ReleaseRes(CORE_NE_API h, char* buf);
    ICTLAP_CORE_NE_DLL void CORE_NE_Exit(CORE_NE_API h);
    ICTLAP_CORE_NE_DLL void CORE_NE_Version();
}
/********************************************************************************/

#endif
