(define-module (miny)
  #:export (miny)

  #:use-module (guix gexp)
  #:use-module (guix packages)
  #:use-module (guix git-download)
  #:use-module (guix build-system gnu)
  #:use-module ((guix licenses)
                #:prefix license:)
  #:use-module (gnu packages gl))


(define (git-vc-files name path)
  (computed-file
   name
   (with-imported-modules
	'((guix build utils))
	#~(begin
	   (use-modules (guix build utils))
	   (copy-recursively #$(local-file path #:recursive? #t) #$output)
	   (chdir #$output)
	   (invoke
		#$(file-append (@ (gnu packages version-control) git) "/bin/git")
		"clean" "-fdX") ;; Remove gitignored files.
	   (delete-file-recursively (string-append #$output "/.git"))))))

(define miny
  (package
    (name "miny")
    (version "0.6.0")
    (source (git-vc-files name (dirname (current-filename))))
    (build-system gnu-build-system)
    (arguments
     `(#:phases (modify-phases %standard-phases
                  (delete 'configure)
                  (replace 'install
                    (lambda* (#:key outputs #:allow-other-keys)
                      (let* ((out (assoc-ref outputs "out"))
                             (bin (string-append out "/bin")))
                        (install-file "miny" bin)))))
       #:tests? #f))
    (inputs (list freeglut))
    (home-page "https://github.com/spacecamper/miny")
    (synopsis "Minesweeper")
    (description "Minesweeper")
    (license license:expat)))
miny
