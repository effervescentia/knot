use crate::{
    ast,
    weak::{ToWeak, WeakRef},
};
use lang::types;

impl ToWeak for ast::Import {
    fn to_weak(&self) -> WeakRef {
        (types::RefKind::Mixed, None)
    }
}
