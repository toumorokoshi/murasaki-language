use std::fmt;
use std::collections::HashMap;
use std::rc::Rc;
use super::types;
use super::types::TypeRef;
use super::Function;
use super::builtins::print;

pub struct LocalObject {
    pub index: usize,
    pub typ: TypeRef
}

impl Clone for LocalObject {
    fn clone(&self) -> LocalObject {
        return LocalObject {
            index: self.index,
            typ: match self.typ {
                // TypeRef::Heap(ref a) => TypeRef::Heap(a.clone()),
                TypeRef::Static(t) => TypeRef::Static(t),
            }
        };
    }
}

pub struct Scope {
    pub locals: HashMap<String, usize>,
    pub functions: HashMap<String, Rc<Function>>,
    pub types: Vec<types::TypeRef>
}

pub struct ScopeInstance {
    pub registers: Vec<i64>
}

impl Scope {
    pub fn new() -> Scope {
        return Scope{
            functions: HashMap::new(),
            locals: HashMap::new(),
            types: Vec::new()
        };
    }

    pub fn get_local(&mut self, name: &String) -> Option<LocalObject> {
        match self.locals.get(name) {
            None => None,
            Some(index) => {
                Some(LocalObject{index: index.clone(), typ: self.types[index.clone()].clone()})
            }
        }
    }

    pub fn add_local(&mut self, name: &String, typ: TypeRef) -> LocalObject {
        let object = self.allocate_local(typ);
        self.locals.insert(name.clone(), object.index);
        return object;
    }

    pub fn add_function(&mut self, name: String, function: Rc<Function>) {
        self.functions.insert(name, function);
    }

    pub fn allocate_local(&mut self, typ: TypeRef) -> LocalObject {
        self.types.push(typ.clone());
        return LocalObject {
            index: self.types.len() - 1,
            typ: typ
        };
    }

    pub fn local_count(&self) -> usize {
        return self.types.len();
    }

    pub fn create_instance(&self) -> ScopeInstance {
        return ScopeInstance{registers: vec![0; self.local_count()]};
    }

    pub fn get_function(&self, name: &String) -> Rc<Function> {
        if let Some(func) = self.functions.get(name) {
            return func.clone();
        } else {
            return Rc::new(Function::NativeFunction {
                name: String::from("print"),
                function: print,
                typ: types::get_none_type()
            });
        }
    }
}

impl fmt::Display for Scope {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        try!(write!(f, "Locals:\n"));
        for (key, value) in &self.locals {
            try!(write!(f, "  {}: {}\n", key, value));
        }
        try!(write!(f, "Types:\n"));
        for typ in &self.types {
            try!(write!(f, "  {}\n", typ));
        }
        return write!(f, "");
    }
}