use crate::ast;
use lang::types;

impl super::ToWeak for ast::Import {
    fn to_weak(&self) -> super::Ref {
        (types::RefKind::Mixed, None)
    }
}
