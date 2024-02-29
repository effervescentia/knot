use crate::ast;
use lang::types::RefKind;

impl super::ToWeak for ast::Import {
    fn to_weak(&self) -> super::Ref {
        (RefKind::Mixed, super::Type::Infer)
    }
}
