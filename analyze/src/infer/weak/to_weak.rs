use super::{
    data::{Data, Inference, Weak},
    ToWeak,
};
use lang::{ast, types::Kind, Fragment, NodeId};

impl ToWeak for Fragment {
    fn to_weak(&self) -> Weak {
        match self {
            Self::Expression(x) => x.to_weak(),
            Self::Statement(x) => x.to_weak(),
            Self::Component(x) => x.to_weak(),
            Self::Parameter(x) => x.to_weak(),
            Self::TypeExpression(x) => x.to_weak(),
            Self::Declaration(x) => x.to_weak(),
            Self::Import(x) => x.to_weak(),
            Self::Module(x) => x.to_weak(),
        }
    }
}

impl ToWeak for ast::Import {
    fn to_weak(&self) -> Weak {
        (
            Kind::Mixed,
            Data::Infer(Inference::Import(
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
            Data::Infer(Inference::Module(self.declarations.clone())),
        )
    }
}

#[cfg(test)]
mod tests {
    use super::{Data, Inference, ToWeak};
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
                Data::Infer(Inference::Import(
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
                Data::Infer(Inference::Module(vec![NodeId(1), NodeId(1)],))
            )
        );
    }
}
