use super::{RefKind, Type, WeakType};
use crate::{
    analyzer::{infer::weak::ToWeak, WeakRef},
    parser::expression::ksx::KSX,
};

impl ToWeak for KSX<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            KSX::Text(..) => (RefKind::Value, WeakType::Strong(Type::String)),

            KSX::Inline(id) => (RefKind::Value, WeakType::Reference(*id)),

            KSX::Fragment(..) => (RefKind::Value, WeakType::Strong(Type::Element)),

            KSX::ClosedElement(..) => (RefKind::Value, WeakType::Strong(Type::Element)),

            KSX::OpenElement(..) => (RefKind::Value, WeakType::Strong(Type::Element)),
        }
    }
}
