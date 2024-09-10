use super::data::{Inference, Type, Weak};
use lang::{ast, types::Kind, Fragment, NodeId};

pub trait ToWeak {
    /// infer the weak type of an AST fragment
    fn to_weak(&self) -> Weak;
}

impl ToWeak for Fragment {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Expression(x) => x.to_weak(),
            Self::Statement(x) => x.to_weak(),
            Self::Attribute(x) => x.to_weak(),
            Self::Component(x) => x.to_weak(),
            Self::Parameter(x) => x.to_weak(),
            Self::TypeExpression(x) => x.to_weak(),
            Self::Declaration(x) => x.to_weak(),
            Self::Import(x) => x.to_weak(),
            Self::Module(x) => x.to_weak(),

            Self::TypeDeclaration(x) => x.to_weak(),
            Self::TypeModule(x) => x.to_weak(),
        }
    }
}

impl ToWeak for ast::Import {
    fn to_weak(&self) -> Weak {
        (
            Kind::Mixed,
            Type::Infer(Inference::Import(
                self.source.clone(),
                self.path.clone(),
                self.alias.clone(),
            )),
        )
    }
}

impl ToWeak for ast::Module<NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Mixed,
            Type::Infer(Inference::Module(self.declarations.clone())),
        )
    }
}
impl ToWeak for ast::TypeModule<NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Mixed,
            Type::Infer(Inference::Module(self.declarations.clone())),
        )
    }
}

#[cfg(test)]
mod tests {
    use super::{Inference, ToWeak, Type};
    use kore::str;
    use lang::{ast, types::Kind, NodeId};

    #[test]
    fn import() {
        assert_eq!(
            ast::Import::new(
                ast::ImportSource::Local,
                vec![str!("foo")],
                Some(str!("Foo"))
            )
            .to_weak(),
            (
                Kind::Mixed,
                Type::Infer(Inference::Import(
                    ast::ImportSource::Local,
                    vec![str!("foo")],
                    Some(str!("Foo"))
                ))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            ast::Module::new(vec![], vec![NodeId(1), NodeId(1)]).to_weak(),
            (
                Kind::Mixed,
                Type::Infer(Inference::Module(vec![NodeId(1), NodeId(1)],))
            )
        );
    }
}
