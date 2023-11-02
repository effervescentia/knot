use crate::{
    infer::weak::{ToWeak, Weak, WeakRef},
    RefKind,
};
use lang::ast::{Import, ImportSource};

impl ToWeak for Import<usize> {
    fn to_weak(&self) -> WeakRef {
        (RefKind::Mixed, Weak::Infer)
    }
}

impl ToWeak for ImportSource {
    fn to_weak(&self) -> WeakRef {
        (RefKind::Mixed, Weak::Infer)
    }
}
