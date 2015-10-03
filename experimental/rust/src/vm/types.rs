use std::sync::Arc;

pub struct Type {
    name: &'static str
}

pub enum TypeRef {
    // we use Arcs because it's thread-safe,
    // and the type references is shared across
    // processes.
    Heap(Arc<Type>),
    Static(&'static Type),
}

impl TypeRef {
    fn name(&self) -> &str {
        return match self {
            &TypeRef::Heap(ref t) => t.name,
            &TypeRef::Static(ref t) => t.name
        }
    }
}

pub static IntType: Type = Type {name: "Int"};

pub fn get_int_type() -> TypeRef {
    return TypeRef::Static(&IntType);
}