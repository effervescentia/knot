use crate::{
    infer::weak::{ToWeak, Weak, WeakRef},
    RefKind,
};
use lang::ast::Import;

impl ToWeak for Import {
    fn to_weak(&self) -> WeakRef {
        (RefKind::Mixed, Weak::Infer)
    }
}
