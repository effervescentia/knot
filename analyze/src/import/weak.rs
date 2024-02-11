use crate::infer::weak::{ToWeak, Weak, WeakRef};
use lang::{ast, types};

impl ToWeak for ast::Import {
    fn to_weak(&self) -> WeakRef {
        (types::RefKind::Mixed, Weak::Infer)
    }
}
