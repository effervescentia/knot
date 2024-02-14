use crate::{
    ast,
    weak::{ToWeak, Weak, WeakRef},
};
use lang::types;

impl ToWeak for ast::Import {
    fn to_weak(&self) -> WeakRef {
        (types::RefKind::Mixed, Weak::Infer)
    }
}
