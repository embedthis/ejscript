import fetch from 'node-fetch';
import crypto from 'crypto';

/**
 * Perform a digest-authenticated fetch for method/url with user/password.
 *
 * @param {string} url 
 * @param {object} options — fetch options (method, headers, body, etc)
 * @param {string} username 
 * @param {string} password 
 */
export async function digestFetch(url, options, username, password) {
  // First request: no Authorization header
  const res1 = await fetch(url, options);
  
  if (res1.status !== 401) {
    // No digest challenge, return response directly
    return res1;
  }
  
  // Extract WWW-Authenticate header
  const wwwAuth = res1.headers.get('www-authenticate');
  if (!wwwAuth || !wwwAuth.startsWith('Digest ')) {
    throw new Error('Server did not issue a Digest challenge');
  }
  
  // Parse the challenge parameters
  const challenge = parseDigestChallenge(wwwAuth.slice(7));
  // console.log('challenge:', challenge);
  
  const method = (options.method || 'GET').toUpperCase();
  const uri = new URL(url).pathname + (new URL(url).search || '');
  
  // Construct client nonce (cnonce)
  const cnonce = crypto.randomBytes(16).toString('hex');
  const nc = '00000001';
  
  // Default to MD5
  const algorithm = (challenge.algorithm || 'MD5').toUpperCase();
  if (algorithm !== 'MD5') {
    throw new Error(`Unsupported algorithm: ${algorithm}`);
  }

  // Compute HA1 = MD5(username:realm:password)
  const ha1 = md5(`${username}:${challenge.realm}:${password}`);
  // Compute HA2 = MD5(method:uri)
  const ha2 = md5(`${method}:${uri}`);
  
  // Compute response = MD5(HA1:nonce:nonceCount:cnonce:qop:HA2)
  if (!challenge.qop) {
    throw new Error('Server challenge did not include qop; this minimal code supports only qop=auth');
  }
  const qop = 'auth';
  const response = md5(`${ha1}:${challenge.nonce}:${nc}:${cnonce}:${qop}:${ha2}`);
  
  // Build Authorization header
  const authHeader = [
    `Digest username="${username}"`,
    `realm="${challenge.realm}"`,
    `nonce="${challenge.nonce}"`,
    `uri="${uri}"`,
    `algorithm=${algorithm}`,
    `response="${response}"`,
    `qop=${qop}`,
    `nc=${nc}`,
    `cnonce="${cnonce}"`
  ];
  if (challenge.opaque) {
    authHeader.push(`opaque="${challenge.opaque}"`);
  }
  const headerValue = authHeader.join(', ');
  
  // Second request: with Authorization
  const opts2 = {
    ...options,
    headers: {
      ...(options.headers || {}),
      Authorization: headerValue
    }
  };
  
  const res2 = await fetch(url, opts2);
  return res2;
}

function parseDigestChallenge(challengeStr) {
  const params = {};
  const regex = /(\w+)=("(?:\\.|[^"])*"|[^\s,]+)/g;
  let match;
  while ((match = regex.exec(challengeStr)) !== null) {
    const key = match[1];
    let val = match[2];
    if (val.startsWith('"') && val.endsWith('"')) {
      val = val.slice(1, -1);
    }
    params[key] = val;
  }
  return params;
}

function md5(data) {
  return crypto.createHash('md5').update(data).digest('hex');
}
