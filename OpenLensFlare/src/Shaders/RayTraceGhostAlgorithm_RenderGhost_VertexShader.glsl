// Structure describing a lens interface
struct Lens
{
    // Center of the lens interface
    vec3 center;
    
    // Refraction indices
    vec3 n;
    
    // Radius of curvature
    float radius;
    
    // Height of the lens element
    float height;
    
    // Aperture height
    float aperture;
    
    // Coating thickness
    float d1;
};

// Structure describing a ray
struct Ray
{
    // Ray position
    vec3 pos;
    
    // Ray direction
    vec3 dir;
    
    // UV coordinates on the aperture
    vec2 uv;
    
    // Relative radius
    float radius;
    
    // Accumulated intensity
    float intensity;
};

// Creates a Lens object ouf of the ith element.
Lens createLens(int id)
{
    Lens result;

    result.center = vLensCenter[id];
    result.n = vLensIor[id];
    result.radius = fLensRadius[id];
    result.height = fLensHeight[id];
    result.aperture = fLensAperture[id];
    result.d1 = fLensCoating[id];

    return result;
}

// Initializes a ray with the given position and direction.
Ray createRay(vec3 rayPosition, vec3 rayDirection)
{
    Ray result;
    
    result.pos = rayPosition;
    result.dir = rayDirection;
    result.uv = vec2(0.0);
    result.radius = 0.0;
    result.intensity = 1.0;
    
    return result;
}

// Structure describing an intersection
struct Intersection
{
    // Position of intersection
    vec3 pos;
    
    // Normal of intersection
    vec3 normal;
    
    // Incident angle
    float theta;
    
    // Whether it was a successful hit
    bool hit;
    
    // Should we invert the ray
    float inverted;
};

// Performs a ray-plane intersection
Intersection intersectPlane(Lens lens, Ray ray)
{
    Intersection i;
    
    // Calculate the point of intersection
    i.pos = ray.pos + (ray.dir * ((lens.center.z - ray.pos.z) / ray.dir.z));
    
    // Calculate the normal of intersection
    i.normal = vec3(0.0, 0.0, ray.dir.z > 0.0 ? -1.0 : 1.0);
    
    // Irrelevant
    i.theta = acos(dot(-ray.dir, i.normal));
    
    // It's always a hit
    i.hit = true;
    
    return i;
}

// Performs a ray-sphere intersection
Intersection intersectSphere(Lens lens, Ray ray)
{
    Intersection i;
    
    // Vector pointing from the ray to the sphere center
    vec3 D = ray.pos - lens.center;
    float B = dot(D, ray.dir);
    float C = dot(D, D) - (lens.radius * lens.radius);
    
    // Discriminant
    float B2_C = B * B - C;
    
    // No hit if the discriminant is negative
    if (B2_C < 0.0)
    {
        i.hit = false;
        return i;
    }
    
    // The ray is inside the virtual sphere if multiplying its Z coordinate by 
    // the lens radius yields a positive value, and it is outside, if the result
    // is negative.
    float inside = sign(lens.radius * ray.dir.z);
    float outside = -inside;

    // '-B - sqrt(B2_C)' if the ray is outside,
    // '-B + sqrt(B2_C)' if the ray is inside.
    float t = -B + sqrt(B2_C) * inside;
    
    // Compute the hit position
    i.pos = ray.pos + t * ray.dir;
    
    // Compute the hit normal - also flip it if the ray is inside the sphere
    i.normal = normalize(i.pos - lens.center) * -inside;
    
    // Compute the hit angle
    i.theta = acos(dot(-ray.dir, i.normal));
    
    // It was a hit
    i.hit = true;
    
    return i;
}

// Fresnel equation
float fresnelAR(float theta0, float lambda, float d, float n0, float n1, float n2)
{
	// Apply Snell's law to get the other angles
	float theta1 = asin(sin(theta0) * n0 / n1);
	float theta2 = asin(sin(theta0) * n0 / n2);

	float rs01 = -sin(theta0 - theta1) / sin(theta0 + theta1);
	float rp01 = tan(theta0 - theta1) / tan(theta0 + theta1);
	float ts01 = 2.0 * sin(theta1) * cos(theta0) / sin(theta0 + theta1);
	float tp01 = ts01 * cos(theta0 - theta1);

	float rs12 = -sin(theta1 - theta2) / sin(theta1 + theta2);
	float rp12 = tan(theta1 - theta2) / tan(theta1 + theta2);

	float ris = ts01 * ts01 * rs12;
	float rip = tp01 * tp01 * rp12;

	float dy = d * n1;
	float dx = tan(theta1) * dy;
	float delay = sqrt(dx * dx + dy * dy);
	float relPhase = 4.0 * PI / lambda * (delay - dx * sin(theta0));

	float out_s2 = rs01 * rs01 + ris * ris + 2.0f * rs01 * ris * cos(relPhase);
	float out_p2 = rp01 * rp01 + rip * rip + 2.0f * rp01 * rip * cos(relPhase);

	return (out_s2 + out_p2) * 0.5;
}

// Traces a ray from the entrance plane up until the sensor.
Ray traceRay(Ray ray)
{    
    // Current phase of testing (0: forward #1, 1: backward, 2: forward #2)
    int phase = 0;
    
    // Tracing direction
    int delta = 1;
    
    for (int t = 1; t < iLength; t += delta)
    {
        // Extract the current lens
        Lens lens = createLens(t);
    
        // Change direction upon reaching the designated interfaces
        bool reflectRay = phase < iNumIndices && t == iGhostIndices[phase];
        if (reflectRay)
        {
            delta = -delta;
            ++phase;
        }
        
        // Determine the intersection
        Intersection i = (lens.radius == 0.0) ? 
            intersectPlane(lens, ray) : 
            intersectSphere(lens, ray);
        
        // Stop tracing if we couldn't hit anything
        if (i.hit == false)
        {
            ray.intensity = 0.0;
            break;
        }
        
        // Update the ray
        ray.pos = i.pos;
        
        // Update the relative radius
        //if (lens.radius != 0.0)
        {
            ray.radius = max(ray.radius, length(ray.pos.xy) / (lens.height));
        }
        
        // Save the UV upon reaching the aperture
        if (lens.aperture != 0.0)
        {
            ray.uv = ray.pos.xy / lens.aperture;
        }
        
        // Don't reflect/refract on flat surfaces
        if (lens.radius == 0.0)
            continue;
        
        // Get the refractive indices
        float n0 = ray.dir.z < 0.0 ? lens.n.x : lens.n.z;
        float n1 = lens.n.y;
        float n2 = ray.dir.z < 0.0 ? lens.n.z : lens.n.x;
        
        // Are we refracting?
        if (!reflectRay)
        {
            // Refract the ray
            ray.dir = refract(ray.dir, i.normal, n0 / n2);
            
            // Stop if we experience total internal reflection
            if (ray.dir == vec3(0.0))
            {
                ray.intensity = 0.0;
                break;
            }
        }
        
        // Or are we reflecting?
        else
        {
            // Reflect the ray
            ray.dir = reflect(ray.dir, i.normal);
            
            // Calculate the Fresnel reflectivity (R) term
            float R = fresnelAR(i.theta, fLambda, lens.d1, n0, n1, n2);
            
            // Update the intensity
            ray.intensity *= R;
        }
    }
    
    // Return the modified ray
    return ray;
}

// Outputs
out vec2 vParam;
out vec2 vPos;
out vec2 vUv;
out float fRadius;
out float fIntensity;

void main()
{
    // Subdivision size, corner position and step size
    int SUBDIVISION = iRayCount - 1;
    vec2 CORNER = vec2(-1.0);
    vec2 STEP = vec2(2.0) / SUBDIVISION;
    
    // Quad indices
    ivec2 QUAD_IDS[6] = ivec2[6]
    (
        ivec2(0, 0),
        ivec2(1, 0),
        ivec2(1, 1),

        ivec2(1, 1),
        ivec2(0, 1),
        ivec2(0, 0)
    );
    
    // Column id
    int col = (gl_VertexID / 6) % SUBDIVISION;
    
    // Row id
    int row = (gl_VertexID / 6) / SUBDIVISION;
    
    // Vertex id
    int vert = gl_VertexID % 6;
    
    // Calculate the vertex position
    vec2 vertexPos = CORNER + (ivec2(col, row) + QUAD_IDS[vert]) * STEP;

    // Calculate the ray position
    vec2 rayPos = vGridSize * vertexPos + vGridCenter;

    // Scale the normalized position by the pupil lens height
    vec2 scaledRayPos = rayPos * fLensHeight[1];
    
    // Generate the ray that we're tracing
    Ray ray = createRay(vec3(scaledRayPos, fRayDistance), vRayDir);
    
    // Result of the trace
    Ray result = traceRay(ray);
    
    // Write out the output values
    vParam = rayPos;
    vUv = result.uv;
    fRadius = result.radius;
    fIntensity = result.intensity;
    
    //  Render mode: projected ghost
    if (iRenderMode == RENDER_MODE_PROJECTED_GHOST)
    {
        vPos = result.pos.xy / (vFilmSize * 0.5);
    }
    
    // Render mode: pupil grid
    else if (iRenderMode == RENDER_MODE_PUPIL_GRID)
    {
        vPos = rayPos;
    }
}