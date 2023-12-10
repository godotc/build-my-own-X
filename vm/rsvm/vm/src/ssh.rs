use futures;
use thrussh::*;
use thrussh_keys::*;

#[derive(Clone, Debug)]
pub struct Server {}

impl server::Server for Server {
    type Handler = Self;

    fn new(&self) -> Self::Handler {
        self.clone()
    }
}

impl server::Handler for Server {
    type Error = std::io::Error;

    type FutureAuth = futures::Finished<(Self, server::Auth), Self::Error>;
    type FutureUnit = futures::Finished<(Self, server::Session), Self::Error>;
    type FutureBool = futures::Finished<(Self, server::Session, bool), Self::Error>;

    fn finished_auth(self, auth: server::Auth) -> Self::FutureAuth {
        futures::finished((self, auth))
    }

    fn finished_bool(self, session: server::Session, b: bool) -> Self::FutureBool {
        futures::finished((self, session, b))
    }

    fn finished(self, session: server::Session) -> Self::FutureUnit {
        futures::finished((self, session))
    }

    fn auth_publickey(
        self,
        _user: &str,
        _public_key: &thrussh_keys::key::PublicKey,
    ) -> Self::FutureAuth {
        futures::finished((self, server::Auth::Accept))
    }

    fn data(
        self,
        channel: thrussh::ChannelId,
        data: &[u8],
        mut session: server::Session,
    ) -> Self::FutureUnit {
        println!(
            "data on channel {:?}: {:?}",
            channel,
            std::str::from_utf8(data)
        );
        session.data(channel, None, data);
        futures::finished((self, session))
    }
}
