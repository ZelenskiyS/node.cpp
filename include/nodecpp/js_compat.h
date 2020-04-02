/* -------------------------------------------------------------------------------
* Copyright (c) 2020, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the OLogN Technologies AG nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL OLogN Technologies AG BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/

#ifndef NODECPP_JS_COMPAT_H
#define NODECPP_JS_COMPAT_H

#include "common.h"
#include <typeinfo>
#include <typeindex>

namespace nodecpp::js {

	class JSObject;
	class JSArray;
	class JSVar
	{
		friend class JSObject;
		friend class JSArray;
		friend nodecpp::string typeOf( const JSVar& );

		enum Type { undef, boolean, num, string, ownptr, softptr, fn };
		Type type = Type::undef;
		static constexpr size_t memsz = sizeof( nodecpp::string ) > 16 ? sizeof( nodecpp::string ) : 16;
		uint8_t basemem[memsz];

		struct FunctionBase
		{
		};

		static constexpr size_t fnSize = sizeof( std::function<double(double)> );
		static_assert( fnSize == 64 );

		using owningptr2jsobj = nodecpp::safememory::owning_ptr<JSObject>;
		using softptr2jsobj = nodecpp::safememory::soft_ptr<JSObject>;
		using owningptr2jsarr = nodecpp::safememory::owning_ptr<JSArray>;
		using softptr2jsarr = nodecpp::safememory::soft_ptr<JSArray>;

		bool* _asBool() { return reinterpret_cast<bool*>( basemem ); }
		double* _asNum() { return reinterpret_cast<double*>( basemem ); }
		nodecpp::string* _asStr() { return reinterpret_cast<nodecpp::string*>( basemem ); }
		owningptr2jsobj* _asOwn() { return reinterpret_cast<owningptr2jsobj*>( basemem ); }
		softptr2jsobj* _asSoft() { return reinterpret_cast<softptr2jsobj*>( basemem ); }

		const bool* _asBool() const { return reinterpret_cast<const bool*>( basemem ); }
		const double* _asNum() const { return reinterpret_cast<const double*>( basemem ); }
		const nodecpp::string* _asStr() const { return reinterpret_cast<const nodecpp::string*>( basemem ); }
		const owningptr2jsobj* _asOwn() const { return reinterpret_cast<const owningptr2jsobj*>( basemem ); }
		const softptr2jsobj* _asSoft() const { return reinterpret_cast<const softptr2jsobj*>( basemem ); }

		void deinit()
		{
			switch ( type )
			{
				case Type::undef:
				case Type::boolean:
				case Type::num:
					break;
				case Type::string:
					_asStr()->~basic_string();
					break;
				case Type::ownptr:
					_asOwn()->~owningptr2jsobj();
					break;
				case Type::softptr:
					_asSoft()->~softptr2jsobj();
					break;
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
			}
			type = Type::undef;
		}
		void init( const JSVar& other )
		{
			deinit();
			type = other.type;
			switch ( other.type )
			{
				case Type::undef:
					break;
				case Type::boolean:
					*_asBool() = *(other._asBool());
					break;
				case Type::num:
					*_asNum() = *(other._asNum());
					break;
				case Type::string:
					*_asStr() = *(other._asStr());
					break;
				case Type::ownptr:
					_asOwn()->~owningptr2jsobj();
					*_asSoft() = *(other._asOwn());
					type = Type::softptr;
					break;
				case Type::softptr:
					*_asSoft() = *(other._asSoft());
					break;
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
			}
		}
		void init( bool b )
		{
			if ( type == Type::boolean )
				*_asBool() = b;
			else
			{
				deinit();
				type = Type::boolean;
				*_asBool() = b;
			}
		}
		void init( double d )
		{
			if ( type == Type::num )
				*_asNum() = d;
			else
			{
				deinit();
				type = Type::num;
				*_asNum() = d;
			}
		}
		void init( const nodecpp::string& str )
		{
			if ( type == Type::string )
				*_asStr() = str;
			else
			{
				deinit();
				type = Type::string;
				new(_asStr())nodecpp::string( str );
			}
		}
		void init( owningptr2jsobj&& ptr )
		{
			if ( type == Type::ownptr )
				*_asOwn() = std::move( ptr );
			else
			{
				deinit();
				type = Type::ownptr;
				new(_asOwn())owningptr2jsobj( std::move( ptr ) );
			}
		}
		void init( const softptr2jsobj ptr )
		{
			if ( type == Type::softptr )
				*_asSoft() = ptr;
			else
			{
				deinit();
				type = Type::softptr;
				new(_asSoft())softptr2jsobj( ptr );
			}
		}
		void init( owningptr2jsarr&& ptr )
		{
			if ( type == Type::ownptr )
				*_asOwn() = std::move( ptr );
			else
			{
				deinit();
				type = Type::ownptr;
				new(_asOwn())owningptr2jsarr( std::move( ptr ) );
			}
		}
		void init( const softptr2jsarr ptr )
		{
			if ( type == Type::softptr )
				*_asSoft() = ptr;
			else
			{
				deinit();
				type = Type::softptr;
				new(_asSoft())softptr2jsarr( ptr );
			}
		}

	public:
		JSVar() {}
		JSVar( const JSVar& other ) { init( other );}
//		JSVar( soft_ptr<JSVar> other ) { init( *other );}
		JSVar( bool b ) { init( b ); }
		JSVar( double d ) { init( d ); }
		JSVar( int n ) { init( (double)n ); }
		JSVar( const nodecpp::string& str ) { init( str ); }
		JSVar( owningptr2jsobj&& ptr ) { init( std::move( ptr ) ); }
		JSVar( const softptr2jsobj ptr ) { init( ptr ); }
		JSVar( owningptr2jsarr&& ptr ) { init( std::move( ptr ) ); }
		JSVar( const softptr2jsarr ptr ) { init( ptr ); }
		JSVar( std::initializer_list<double> l ) { auto arr = make_owning<JSArray>(l); init( std::move( arr ) ); }
		JSVar( std::initializer_list<int> l ) { auto arr = make_owning<JSArray>(l); init( std::move( arr ) ); }

		~JSVar() { deinit(); }

	public:
		JSVar& operator = ( const JSVar& other ) { init( other ); return *this; }
//		JSVar& operator = ( soft_ptr<JSVar> other ) { init( *other ); return *this; }
		JSVar& operator = ( bool b ) { init( b ); return *this; }
		JSVar& operator = ( double d ) { init( d ); return *this; }
		JSVar& operator = ( const nodecpp::string& str ) { init( str ); return *this; }
		JSVar& operator = ( owningptr2jsobj&& ptr ) { init( std::move( ptr ) ); return *this; }
		JSVar& operator = ( softptr2jsobj ptr ) { init( ptr ); return *this; }
		JSVar& operator = ( owningptr2jsarr&& ptr ) { init( std::move( ptr ) ); return *this; }
		JSVar& operator = ( softptr2jsarr ptr ) { init( ptr ); return *this; }

		JSVar operator [] ( const JSVar& var );
		JSVar operator [] ( size_t idx );
		JSVar operator [] ( int idx ) { return operator [] ((size_t)idx); }
		JSVar operator [] ( const nodecpp::string& key );
		JSVar operator [] ( const char* key ) { nodecpp::string s(key); return operator [] (s); }

		nodecpp::string toString() const;
		bool operator !() const
		{
			// TODO: make sure we report right values!!!
			switch ( type )
			{
				case Type::undef:
					return true; 
				case Type::boolean:
					return !*_asBool();
				case Type::num:
					return _asNum() == 0;
				case Type::string:
					return _asStr()->size() == 0 || *_asStr() == "false";
				case Type::ownptr:
					return *_asOwn() != nullptr;
					break;
				case Type::softptr:
					return *_asSoft() != nullptr;
				default:
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
			}
		}
		operator nodecpp::string () const { return toString(); }

		bool has( const JSVar& other ) const;
		bool has( size_t idx ) const;
		bool has( double num ) const;
		bool has( nodecpp::string str ) const;

		bool in( const JSVar& collection ) const { return collection.has( *this ); }
	};

	inline bool jsIn( const JSVar& var, const JSVar& collection )  { return collection.has( var ); }
	inline bool jsIn( size_t idx, const JSVar& collection )  { return collection.has( idx ); }
	inline bool jsIn( nodecpp::string str, const JSVar& collection )  { return collection.has( str ); }

	inline
	nodecpp::string typeOf( const JSVar& var )
	{
		switch ( var.type )
		{
			case JSVar::Type::undef:
				return "undefined";
			case JSVar::Type::boolean:
				return "boolean";
			case JSVar::Type::num:
				return "number";
			case JSVar::Type::string:
				return "string";
			case JSVar::Type::ownptr:
				return "object";
			case JSVar::Type::softptr:
				return "object";
			case JSVar::Type::fn:
				return "function";
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)(var.type) ); 
		}
	}

	class JSObject
	{
	protected:
		nodecpp::map<nodecpp::string, JSVar> pairs;
	
		void toString_( nodecpp::string& ret, const nodecpp::string offset, const nodecpp::string separator ) const { 
			if ( pairs.size() == 0 )
				return;
			for ( auto& entry : pairs )
			{
				nodecpp::string entrystr = nodecpp::format( "{}{}: {}{}", offset, entry.first, entry.second.toString(), separator );
				ret += entrystr;
			}
			ret.erase( ret.size() - separator.size(), separator.size() );
		}

		JSVar findOrAdd ( nodecpp::string s ) {
			auto f = pairs.find( s );
			if ( f != pairs.end() )
				return f->second;
			else
			{
				auto insret = pairs.insert( std::make_pair( s, JSVar() ) );
				return insret.first->second;
			}
		}

	public:
		JSObject() {}
		JSObject(std::initializer_list<std::pair<nodecpp::string, JSVar>> l)
		{
			for ( auto& p : l )
				pairs.insert( std::move(*const_cast<std::pair<nodecpp::string, JSVar>*>(&p)) );
		}
	public:
		virtual ~JSObject() {}
		static owning_ptr<JSObject> makeJSObject() { return make_owning<JSObject>(); }
		static owning_ptr<JSObject> makeJSObject(std::initializer_list<std::pair<nodecpp::string, JSVar>> l) { return make_owning<JSObject>(l); }
		virtual JSVar operator [] ( const JSVar& var )
		{
			nodecpp::string s = var.toString(); // TODO: revise implementation!!!
			return findOrAdd( s );
		}
		virtual JSVar operator [] ( size_t idx )
		{
			nodecpp::string s = nodecpp::format( "{}", idx );
			return findOrAdd( s );
		}
		virtual JSVar operator [] ( const nodecpp::string& key )
		{
			return findOrAdd( key );
		}
		virtual JSVar operator [] ( const nodecpp::string_literal& key ) {
			nodecpp::string s( key.c_str() );
			return findOrAdd( s );
		}
		virtual JSVar operator [] ( const char* key ) {
			nodecpp::string s( key );
			return findOrAdd( s );
		}
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "{ \n";
			toString_( ret, "  ", ",\n" );
			ret += " }";
			return ret; 
		}
		virtual void forEach( std::function<void(nodecpp::string)> cb )
		{
			for ( auto& e: pairs )
				cb( e.first );
		}
		virtual bool has( const JSVar& var ) const
		{
			auto f = pairs.find( var.toString() );
			return f != pairs.end();
		}
		virtual bool has( size_t idx ) const
		{
			auto f = pairs.find( nodecpp::format( "{}", idx ) );
			return f != pairs.end();
		}
		virtual bool has( double num ) const
		{
			auto f = pairs.find( nodecpp::format( "{}", num ) );
			return f != pairs.end();
		}
		virtual bool has( nodecpp::string str ) const
		{
			auto f = pairs.find( str );
			return f != pairs.end();
		}
	};

	class JSArray : public JSObject
	{
		nodecpp::vector<JSVar> elems;
	public:
		JSArray() {}
		JSArray(std::initializer_list<JSVar> l)
		{
			for ( auto& p : l )
				elems.push_back( std::move(*const_cast<JSVar*>(&p)) );
		}
		JSArray(std::initializer_list<double> l)
		{
			for ( auto& d : l )
				elems.push_back( JSVar( d ) );
		}
		JSArray(std::initializer_list<int> l)
		{
			for ( auto& d : l )
				elems.push_back( JSVar( d ) );
		}
		JSArray(std::initializer_list<nodecpp::string> l)
		{
			for ( auto& str : l )
				elems.push_back( JSVar( str ) );
		}
	public:
		static owning_ptr<JSArray> makeJSArray() { return make_owning<JSArray>(); }
		static owning_ptr<JSArray> makeJSArray(std::initializer_list<JSVar> l) { return make_owning<JSArray>(l); } // TODO: ownership of args
		static owning_ptr<JSArray> makeJSArray(std::initializer_list<double> l) { return make_owning<JSArray>(l); }
		virtual JSVar operator [] ( const JSVar& var )
		{
			// TODO: revise implementation!!!
			if ( var.type == JSVar::Type::num )
				return operator [] ( *(var._asNum()) );
			nodecpp::string s = var.toString(); 
			return findOrAdd( s );
		}
		virtual JSVar operator [] ( size_t idx )
		{
			if ( idx < elems.size() )
				return elems[ idx ];
			else
			{
				elems.reserve( idx + 1 );
				for ( size_t i=elems.size(); i<elems.capacity(); ++i )
					elems.push_back( JSVar() );
				return elems[ idx ];
			}
		}
		virtual JSVar operator [] ( const nodecpp::string& strIdx )
		{
			if ( strIdx.size() && strIdx[0] >= '0' && strIdx[0] <= '9' )
			{
				char* end;
				size_t idx = strtol( strIdx.c_str(), &end, 10 );
				if ( end - strIdx.c_str() == strIdx.size() )
					return operator [] ( idx );
			}
			return JSObject::operator[](strIdx);
		}
		virtual nodecpp::string toString() const { 
			nodecpp::string ret = "[ ";
			if ( pairs.size() )
			{
				toString_( ret, "", ", " );
				for ( auto& x : elems )
					ret += nodecpp::format( ", {}", x.toString() );
			}
			else if ( elems.size() )
			{
				for ( auto& x : elems )
					ret += nodecpp::format( "{}, ", x.toString() );
				ret.erase( ret.end() - 2 );
			}
			ret += " ]";
			return ret; 
		}
		virtual bool has( size_t idx ) const
		{
			auto f = pairs.find( nodecpp::format( "{}", idx ) );
			return f != pairs.end();
		}
	};


	inline
	JSVar JSVar::operator [] ( const JSVar& var )
	{
		switch ( type )
		{
			case Type::undef:
				init( JSObject::makeJSObject() );
				return (*_asOwn())->operator[]( var );
			case Type::boolean:
			case Type::num:
				return JSVar();
			case Type::string:
			{
				if ( var.type == Type::num )
				{
					auto pstr = _asStr();
					size_t idx = *( var._asNum() );
					if ( idx < pstr->size() )
						return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
					else
						return JSVar(); // TODO: mignt be something else!!!
				}
				else if ( var.type == Type::string )
				{
					const nodecpp::string& str = *(var._asStr());
					if ( str.size() && str[0] >= '0' && str[0] <= '9' )
					{
						auto pstr = _asStr();
						char* end;
						size_t idx = strtol( str.c_str(), &end, 10 );
						if ( end - str.c_str() == str.size() )
							return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
						else
							return JSVar(); // TODO: mignt be something else!!!
					}
					else
						return JSVar(); // TODO: mignt be something else!!!
				}
				else
					return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::ownptr:
				return (*_asOwn())->operator[]( var );
			case Type::softptr:
				return (*_asSoft())->operator[]( var );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
					return JSVar(); // TODO: mignt be something else!!!
		}
	}

	inline
	JSVar JSVar::operator [] ( size_t idx )
	{
		switch ( type )
		{
			case Type::undef:
				init( JSObject::makeJSObject() );
				return (*_asOwn())->operator[]( idx );
			case Type::boolean:
			case Type::num:
				return JSVar(); // TODO: mignt be something else!!!
			case Type::string:
			{
				auto pstr = _asStr();
				if ( idx < pstr->size() )
					return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
				else
				return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::ownptr:
				return (*_asOwn())->operator[]( idx );
			case Type::softptr:
				return (*_asSoft())->operator[]( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar(); // TODO: mignt be something else!!!
		}
	}

	inline
	JSVar JSVar::operator [] ( const nodecpp::string& key )
	{
		switch ( type )
		{
			case Type::undef:
				init( JSObject::makeJSObject() );
				return (*_asOwn())->operator[]( key );
			case Type::boolean:
			case Type::num:
				return JSVar(); // TODO: mignt be something else!!!
			case Type::string:
			{
				if ( key.size() && key[0] >= '0' && key[0] <= '9' )
				{
					auto pstr = _asStr();
					char* end;
					size_t idx = strtol( key.c_str(), &end, 10 );
					if ( end - key.c_str() == key.size() )
						return JSVar( nodecpp::string( pstr->substr( idx, 1 ) ) ); // TODO: ownership
				}
				return JSVar(); // TODO: mignt be something else!!!
			}
			case Type::ownptr:
				return (*_asOwn())->operator[]( key );
			case Type::softptr:
				return (*_asSoft())->operator[]( key );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return JSVar(); // TODO: mignt be something else!!!
		}
	}

	inline
	nodecpp::string JSVar::toString() const
	{
		switch ( type )
		{
			case Type::undef:
				return nodecpp::string( "undefined" );
			case Type::boolean:
				return nodecpp::format( "{}", *_asBool() );
			case Type::num:
				return nodecpp::format( "{}", *_asNum() );
			case Type::string:
				return nodecpp::format( "{}", *_asStr() );
			case Type::ownptr:
				return (*_asOwn())->toString();
			case Type::softptr:
				return (*_asSoft())->toString();
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return nodecpp::string( "undefined" );
				break;
		}
	}

	inline
	bool JSVar::has( const JSVar& other ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::ownptr:
				return (*_asOwn())->has( other );
			case Type::softptr:
				return (*_asSoft())->has( other );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	inline
	bool JSVar::has( size_t idx ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::ownptr:
				return (*_asOwn())->has( idx );
			case Type::softptr:
				return (*_asSoft())->has( idx );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	inline
	bool JSVar::has( double num ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::ownptr:
				return (*_asOwn())->has( num );
			case Type::softptr:
				return (*_asSoft())->has( num );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	inline
	bool JSVar::has( nodecpp::string str ) const
	{
		switch ( type )
		{
			case Type::undef:
			case Type::boolean:
			case Type::num:
				return false;
			case Type::string:
				return false; // TODO: ensure we report a right value
			case Type::ownptr:
				return (*_asOwn())->has( str );
			case Type::softptr:
				return (*_asSoft())->has( str );
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type: {}", (size_t)type ); 
				return false;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	class JSModule
	{
	public:
		virtual ~JSModule() {}
	};

	template<auto value>
	struct JSModule2JSVar : public JSModule {};

	template<typename Class, typename Result, Result Class::* member>
	struct JSModule2JSVar<member> : public Class
	{
		//JSVar& operator = ( soft_ptr<JSVar> other ) { (this->*member)->operator = ( other ); return (this->*member); } // TODO: ensure necessity and semantic
		JSVar& operator = ( bool b ) { (this->*member)->operator = ( b ); return (this->*member); }
		JSVar& operator = ( double d ) { (this->*member)->operator = ( d ); return (this->*member); }
		JSVar& operator = ( const nodecpp::string& str ) { (this->*member)->operator = ( str ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::owning_ptr<JSObject>&& ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::soft_ptr<JSObject> ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::owning_ptr<JSArray>&& ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }
		JSVar& operator = ( nodecpp::safememory::soft_ptr<JSArray> ptr ) { (this->*member)->operator = ( ptr ); return (this->*member); }

		operator JSVar () { return (this->*member); }

		JSVar operator [] ( size_t idx ) { return (this->*member)->operator [] (idx ); }
		JSVar operator [] ( const nodecpp::string& key ) { return (this->*member)->operator [] (key ); }

//		void add( nodecpp::string s, owning_ptr<JSVar>&& var ) { return (this->*member)->add( s, std::move( var ) ); }

		operator nodecpp::string () const  { return (this->*member)->operator nodecpp::string (); }
		bool operator !() const  { return (this->*member)->operator ! (); }

		bool has( const JSVar& other ) const { return (this->*member)->has(other ); }
		bool has( size_t idx ) const { return (this->*member)->has(idx ); }
		bool has( double num ) const { return (this->*member)->has(num ); }
		bool has( nodecpp::string str ) const { return (this->*member)->has(str ); }

		bool in( const JSVar& collection ) const { return (this->*member)->in(collection ); }

		nodecpp::string toString() const { return (this->*member)->toString();}
	};

	class JSModuleMap
	{
		using MapType = nodecpp::map<std::type_index, owning_ptr<JSModule>>;
#ifndef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		MapType _classModuleMap;
		MapType& classModuleMap() { return _classModuleMap; }
#else
		uint8_t mapbytes[sizeof(MapType)];
		MapType& classModuleMap() { return *reinterpret_cast<MapType*>(mapbytes); }
#endif
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> getJsModuleExported_( std::type_index idx )
		{
			auto pattern = classModuleMap().find( idx );
			if ( pattern != classModuleMap().end() )
			{
				nodecpp::safememory::soft_ptr<JSModule> svar0 = pattern->second;
				nodecpp::safememory::soft_ptr<UserClass> svar = soft_ptr_static_cast<UserClass>(svar0);
				return std::make_pair( true, svar );
			}
			else
				return std::make_pair( false, nodecpp::safememory::soft_ptr<UserClass>() );
		}
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> addJsModuleExported_( std::type_index idx, nodecpp::safememory::owning_ptr<JSModule>&& pvar )
		{
			auto check = classModuleMap().insert( std::make_pair( idx, std::move( pvar ) ) );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, check.second, "failed to insert exported value to map; insertion already done for this type" ); 
//			nodecpp::safememory::soft_ptr<UserClass> svar = check.first->second;
			nodecpp::safememory::soft_ptr<JSModule> svar0 = check.first->second;
			nodecpp::safememory::soft_ptr<UserClass> svar = soft_ptr_static_cast<UserClass>(svar0);
			return std::make_pair( true, svar );
		}
	public:
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		void init()
		{
			new(&(classModuleMap()))MapType();
		}
		void destroy()
		{
			classModuleMap().~MapType();
		}
#endif
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> getJsModuleExported()
		{
			return getJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)) );
		}
		template<class UserClass>
		std::pair<bool, nodecpp::safememory::soft_ptr<UserClass>> addJsModuleExported( nodecpp::safememory::owning_ptr<JSModule>&& pvar )
		{
			return addJsModuleExported_<UserClass>( std::type_index(typeid(UserClass)), std::move( pvar ) );
		}
	};
	extern thread_local JSModuleMap jsModuleMap;

	template<class T>
	T& require()
	{
		auto trial = jsModuleMap.getJsModuleExported<T>();
		if ( trial.first )
			return *(trial.second);
		owning_ptr<JSModule> pt = nodecpp::safememory::make_owning<T>();
		auto ret = jsModuleMap.addJsModuleExported<T>( std::move( pt ) );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret.first ); 
		return *(ret.second);
	}

} //namespace nodecpp::js

#endif // NODECPP_JS_COMPAT_H