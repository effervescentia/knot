pub mod transpile;

pub enum JavaScriptModule {
    /// ECMAScript modules
    Modern,

    /// CommonJS modules
    Classic,
}

impl Default for JavaScriptModule {
    fn default() -> Self {
        Self::Modern
    }
}

pub enum TargetFormat {
    JavaScript(JavaScriptModule),
}

enum Command {
    Transpile(TargetFormat),
}
