use crate::{ast, CanonicalId, Node, Range};

pub trait Identify {
    fn id(&self) -> &CanonicalId;
}

impl Identify for CanonicalId {
    fn id(&self) -> &CanonicalId {
        self
    }
}

impl Identify for ast::typed::Meta {
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl Identify for (Range, ast::typed::Meta) {
    fn id(&self) -> &CanonicalId {
        self.1.id()
    }
}

impl<Value, Meta> Identify for Node<Value, Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.2.id()
    }
}

impl<Meta> Identify for ast::meta::Expression<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::Statement<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::Component<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::TypeExpression<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::Parameter<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::Declaration<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::Import<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::Module<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::TypeDeclaration<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}

impl<Meta> Identify for ast::meta::TypeModule<Meta>
where
    Meta: Identify,
{
    fn id(&self) -> &CanonicalId {
        self.0.id()
    }
}
