/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2010 Jose L. Hidalgo Valiño (PpluX)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
	
	Jose L. Hidalgo (www.pplux.com)
	pplux@pplux.com
*/



#ifndef __SLB_CLASS_INFO__
#define __SLB_CLASS_INFO__

#include "Export.hpp"
#include "Object.hpp"
#include "Instance.hpp"
#include "Table.hpp"
#include "ref_ptr.hpp"
#include "FuncCall.hpp"
#include "String.hpp"
//#include "ClassHelpers.hpp"
#include <typeinfo>
#include <map>
#include <vector>

struct lua_State;

namespace SLB {

	class SLB_EXPORT Namespace : public Table
	{
	public:
		Namespace( bool cacheable = true ) : Table("::", cacheable) {}
	protected:
		virtual ~Namespace() {}
	};

	class SLB_EXPORT ClassInfo : public Namespace
	{
	public:
		typedef SLB_Map(TypeInfoWrapper, ref_ptr<ClassInfo> ) BaseClassMap;

		ClassInfo(Manager *m, const TypeInfoWrapper &);


		const TypeInfoWrapper &getTypeid() const { return _typeid; }
		const String &getName() const      { return _name; }
		void setName(const String&);

		void push_ref(lua_State *L, void *ref);
		void push_ptr(lua_State *L, void *ptr, bool fromConstructor = false);
		void push_const_ptr(lua_State *L, const void *const_ptr);
		void push_copy(lua_State *L, const void *ptr);
		void* get_ptr(lua_State*, int pos) const;
		const void* get_const_ptr(lua_State*, int pos) const;

		// Uses dynamic_cast to convert from Base to Derived
		template<class This, class Base>
		void inheritsFrom();

		// This version uses static cast instead of dynamic_cast
		template<class This, class Base>
		void staticInheritsFrom();


		void setConstructor( FuncCall *constructor );
		void setInstanceFactory( InstanceFactory *);

		/** __index method will receive:
		 *  - object
		 *  - key */
		void setObject__index( FuncCall* );

		/** __index method will receive:
		 *  - object
		 *  - key
		 *  - value */
		void setObject__newindex( FuncCall* );

		/** Here you can use setCache/getCache methods to
		 * speed up indexing.
		 *
		 * __index method will receive:
		 *  - [table] -> ClassInfo
		 *  - key */
		void setClass__index( FuncCall* );

		/** Here you can use setCache/getCache methods to
		 * speed up indexing.
		 * __index method will receive:
		 *  - [table] -> ClassInfo
		 *  - key
		 *  - value */
		void setClass__newindex( FuncCall* );

		/** __eq method will receive to objects, and should return
		  * true or false if those objects are equal or not. */
		void set__eq( FuncCall* );

		//This is used by some default initializations...
		bool initialized() const { return _instanceFactory != 0; }

		bool isSubClassOf( const ClassInfo* );
		bool hasConstructor() const { return _constructor.valid(); }

		//--Private methods -(not meant to be used)-------------------
		void setHybrid() { _isHybrid = true; }
		FuncCall* getConstructor() { return _constructor.get(); }

	protected:
		virtual ~ClassInfo();
		void pushImplementation(lua_State *);
		virtual int __index(lua_State*);
		virtual int __newindex(lua_State*);
		virtual int __call(lua_State*);
		virtual int __garbageCollector(lua_State*);
		virtual int __tostring(lua_State*);
		virtual int __eq(lua_State *L);

		Manager          *_manager;
		TypeInfoWrapper   _typeid;
		String            _name;
		InstanceFactory  *_instanceFactory;
		BaseClassMap      _baseClasses;
		ref_ptr<FuncCall> _constructor;
		ref_ptr<FuncCall> _meta__index[2];    // 0 = class, 1 = object
		ref_ptr<FuncCall> _meta__newindex[2]; // 0 = class, 1 = object
		ref_ptr<FuncCall> _meta__eq; 
		bool _isHybrid;

	private:
		void pushInstance(lua_State *L, InstanceBase *instance);
		InstanceBase* getInstance(lua_State *L, int pos) const;

		friend class Manager;
	};


	//--------------------------------------------------------------------
	// Inline implementations:
	//--------------------------------------------------------------------
	
		
	template<class D, class B>
	inline void ClassInfo::inheritsFrom()
	{
		_manager->template addConversor<D,B>();
		_baseClasses[ _TIW(B) ] = _manager->getOrCreateClass(_TIW(B));
	}

	template<class D, class B>
	inline void ClassInfo::staticInheritsFrom()
	{
		_manager->template addStaticConversor<D,B>();
		_baseClasses[ _TIW(B) ] = _manager->getOrCreateClass(_TIW(B));
	}

}


#endif
